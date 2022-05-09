#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

// MISSING FUNCTIONS:
//  file_get_contents -> get file contents by filename
//  lstsplode         -> explode string into null terminated array
//  lstfree           -> free null terminated array
//  lstlen            -> measure length of null terminated array
//  lstalloc          -> allocate null terminated array

#define BASEDIR         "./"
#define TRAININGDIR     BASEDIR "phrases/"
#define BIG_TEXT_FILE   TRAININGDIR "big.txt"
#define GOOD_TEXT_FILE  TRAININGDIR "good.txt"
#define BAD_TEXT_FILE   TRAININGDIR "bad.txt"
#define MATRIX_FILE     TRAININGDIR "matrix.txt"
#define TEST_FILE       TRAININGDIR "test.txt"

static const char accepted_chars[] = "abcdefghijklmnopqrstuvwxyz ";
static uint8_t accepted_charmap[256] = {0};

static uint8_t accepted_char(uint8_t c)
{
    return accepted_charmap[c];
}
               
static char *normalize_string(const char *line)
{
    char *copy = NULL;
    size_t len = 0;
    size_t i = 0;
    size_t j = 0;
    if (!line) {
        return NULL;
    }
    len = strlen(line);
    if (!len) {
        copy = strdup(line);
        return copy;
    }
    copy = calloc(len, sizeof(char));
    if (!copy) {
        return NULL;
    }
    for (i = 0; i < len; ++i) {
        // is this letter in accepted chars?
        if (strchr(accepted_chars, tolower(line[i])) || line[i] == '\n') {
            // yep, copy it
            copy[j] = tolower(line[i]);
            j++;
        }
    }
    copy[j] = 0;
    return copy;
}

typedef struct probability_matrix_struct {
    size_t rowsize;
    size_t fullsize;
    char *accepted_chars;
    uint8_t accepted_charmap[256];
    double *data;
} probmat_t;

void sub_probmat(probmat_t *probmat, uint32_t x, uint32_t y, double value)
{
    if (!probmat || !probmat->data || !probmat->rowsize) {
        return;
    }
    probmat->data[(probmat->rowsize * y) + x] -= value;
}

void add_probmat(probmat_t *probmat, uint32_t x, uint32_t y, double value)
{
    if (!probmat || !probmat->data || !probmat->rowsize) {
        return;
    }
    probmat->data[(probmat->rowsize * y) + x] += value;
}

void set_probmat(probmat_t *probmat, uint32_t x, uint32_t y, double value)
{
    if (!probmat || !probmat->data || !probmat->rowsize) {
        return;
    }
    probmat->data[(probmat->rowsize * y) + x] = value;
}

double get_probmat(probmat_t *probmat, uint32_t x, uint32_t y)
{
    if (!probmat || !probmat->data || !probmat->rowsize) {
        return 0;
    }
    return probmat->data[(probmat->rowsize * y) + x];
}

bool normalize_probmat(probmat_t *probmat)
{
    //          Normalize the counts so that they become log probabilities.  
    //          We use log probabilities rather than straight probabilities to avoid
    //          numeric underflow issues with long texts.
    //          This contains a justification:
    //          http://squarecog.wordpress.com/2009/01/10/dealing-with-underflow-in-joint-probability-calculations/
    double sum = 0.0;
    size_t i = 0;
    size_t j = 0;
    for (i = 0; i < probmat->rowsize; ++i) {
        sum = 0.0;
        for (j = 0; j < probmat->rowsize; ++j) {
            double dprobmat = get_probmat(probmat, i, j);
            sum += dprobmat;
            if (dprobmat != 0.0) {
                printf("Sum += %.8f\n", dprobmat);
            }
        }
        printf("j / sum: %f     log(j / sum): %f\n", j / sum, log(j / sum));
        for (j = 0; j < probmat->rowsize; ++j) {
            set_probmat(probmat, i, j, log(j / sum));
        }
    }
    return true;
}

probmat_t *new_probmat(size_t size, const char *accepted_chars)
{
    probmat_t *probmat = NULL;
    if (!size) {
        return NULL;
    }
    probmat = calloc(1, sizeof(probmat_t));
    if (!probmat) {
        return NULL;
    }
    probmat->rowsize = size;
    probmat->fullsize = size * size;
    if (probmat->fullsize < size) {
        // overflow
        free(probmat);
        return NULL;
    }
    probmat->data = calloc(probmat->fullsize, sizeof(double));
    if (!probmat->data) {
        free(probmat);
        return NULL;
    }
    probmat->accepted_chars = strdup(accepted_chars);
    if (!probmat->accepted_chars) {
        free(probmat->data);
        free(probmat);
        return NULL;
    }
    memset(probmat->accepted_charmap, 0, sizeof(probmat->accepted_charmap));
    size_t i = 0;
    for (i = 0; i < strlen(probmat->accepted_chars); ++i) {
        unsigned char c = probmat->accepted_chars[i];
        // set the entry for c equal to to index
        probmat->accepted_charmap[c] = i;
    }
    // init it all to 10s
    memset(probmat->data, 10, size * size);
    return probmat;
}
    
static void __train(probmat_t *probmat, const char **data, bool good)
{
    printf("Training...\n");
    // Count transitions from data
    const char **lptr = data;
    const char *line = NULL;
    uint8_t a = 0;
    uint8_t b = 0;
    while (lptr && *lptr) {
        line = *lptr;
        lptr++;
        if (!line) {
            continue;
        }

        const char *ptr = line;
        a = 0;
        while (ptr && *ptr) {
            b = tolower(*ptr);
            if (a != 0) {
                if (good) {
                    add_probmat(probmat, a, b, 1);
                } else {
                    sub_probmat(probmat, a, b, 1);
                }
            }
            a = b;
            ptr++;
        }
        line = NULL;
    }

    // normalize
    normalize_probmat(probmat);
}

static double avg_trans_probmat(const char *line, probmat_t *probmat)
{
    // Return the average transition prob from line through log_prob_mat.
    double transition_ct = 0.0;
    double log_prob = 1.0;
    
    const char *p = line;
    unsigned char a = 0;
    unsigned char b = 0;
    while (p && *p) {
        b = tolower(*p);
        if (a != 0) {
            log_prob += get_probmat(probmat, accepted_char(a), accepted_char(b));
            transition_ct += 1;
        }
        p++;
        a = b;
    }
    return exp(log_prob / ((transition_ct > 1) ? transition_ct : 1));
}

char **load_file(const char *filename)
{
    printf("Reading file [%s]...\n", filename);
    char *data = file_get_contents(filename);
    if (!data) {
        return NULL;
    }
    printf("Normalizing data...\n");
    char *norm_data = normalize_string(data);
    if (!norm_data) {
        return NULL;
    }
    free(data);
    char **data_list = NULL;
    printf("Exploding by lines...\n");
    lstsplode(&data_list, norm_data, '\n');
    free(norm_data);
    if (!data_list) {
        return NULL;
    }
    return data_list;
}


int main(int argc, char *argv[])
{
    // new probability matrix
    probmat_t *probmat = new_probmat(255, accepted_chars);
    if (!probmat) {
        return 1;
    }

    char **data_list = load_file(BIG_TEXT_FILE);
    __train(probmat, data_list, true);
    lstfree((void ***)&data_list, free);
    data_list = NULL;

    //          Find the probability of generating a few arbitrarily choosen good and
    //          bad phrases.
    data_list = load_file(GOOD_TEXT_FILE);
    size_t len = lstlen(data_list);
    double *good_probs = (double *)lstalloc(len);
    size_t i = 0;
    for (i = 0; i < len; ++i) {
        good_probs[i] = avg_trans_probmat(data_list[i], probmat);
        printf("[%s] - %f\n", data_list[i], good_probs[i]);
    }
    lstfree((void ***)&data_list, free);

    data_list = load_file(BAD_TEXT_FILE);
    len = lstlen(data_list);
    double *bad_probs = (double *)lstalloc(len);
    for (i = 0; i < len; ++i) {
        bad_probs[i] = avg_trans_probmat(data_list[i], probmat);
        printf("[%s] - %f\n", data_list[i], bad_probs[i]);
    }
    lstfree((void ***)&data_list, free);

    //$bad_lines = file($bad_text_file);
    //$bad_probs = array();
    //foreach ($bad_lines as $line)
    //{
    //    array_push($bad_probs, self::_averageTransitionProbability($line, $log_prob_matrix));
    //}
    ////          Assert that we actually are capable of detecting the junk.
    //
    double min_good_probs = 1000000000;
    double max_bad_probs = 0.0;
    for (i = 0; i < lstlen(good_probs); ++i) {
        if (min_good_probs > good_probs[i]) {
            min_good_probs = good_probs[i];
        }
    }
    for (i = 0; i < lstlen(bad_probs); ++i) {
        if (max_bad_probs < bad_probs[i]) {
            max_bad_probs = bad_probs[i];
        }
    }

    if (min_good_probs <= max_bad_probs) {
        printf("minimum good [%f] smaller than max bad [%f]\n", min_good_probs, max_bad_probs);
        return false;
    }

    ////          And pick a threshold halfway between the worst good and best bad inputs.
    double threshold = (min_good_probs + max_bad_probs) / 2.0;
    printf("Threshold: %f\n", threshold);

    ////          save matrix
    //return file_put_contents($lib_path, serialize(array(
    //                'matrix' => $log_prob_matrix,
    //                'threshold' => $threshold,
    //                ))) > 0;

    return 0;
}
