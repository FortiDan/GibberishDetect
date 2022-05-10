#include "ProbabilityMatrix.h"

using namespace std;

#define ROW_SIZE 256
#define FULL_SIZE (ROW_SIZE * ROW_SIZE)

ProbabilityMatrix::ProbabilityMatrix() :
    m_accepted_chars(),
    m_accepted_charmap(),
    m_data(nullptr)
{
}

bool ProbabilityMatrix::init(const string &accepted)
{
    if (accepted.length() > 256) {
        // ERROR: impossible
        return false;
    }
    m_accepted_chars = accepted;

    m_data = new double[FULL_SIZE];
    if (!m_data) {
        // ERROR: allocation
        return false;
    }
    memset(m_accepted_charmap, 0, sizeof(m_accepted_charmap));
    size_t i = 0;
    // map the accepted characters to their indexes in the m_accepted_chars string
    for (i = 0; i < m_accepted_chars.size(); ++i) {
        // first grab the letter from the accepted characters string
        unsigned char c = m_accepted_chars[i];
        // then store the index into the accepted character map
        m_accepted_charmap[c] = (uint8_t)i;
    }
    // init the matrix to all 10s to start
    for (i = 0; i < FULL_SIZE; ++i) {
        m_data[i] = 10.0;
    }
    return true;
}

void ProbabilityMatrix::train(const vector<string> &data, bool good)
{
    printf("Training...\n");
    // Count transitions from data
    uint8_t a = 0;
    uint8_t b = 0;

    for (auto line = data.begin(); line != data.end(); ++line) {
        a = 0;
        const char *ptr = (*line).c_str();
        while (ptr && *ptr) {
            b = *ptr;
            if (a != 0) {
                if (good) {
                    add(a, b, 1);
                }
                else {
                    sub(a, b, 1);
                }
            }
            a = b;
            ptr++;
        }
    }
    
    // normalize the matrix values
    normalize();
}

// calculate the average transition probability for a line, the transition
// probability is how likely any given letter is to transition to the next
double ProbabilityMatrix::avg_trans(const string &line) const
{
    // Return the average transition prob from line through log_prob_mat.
    double transition_ct = 0.0;
    double log_prob = 1.0;

    const char *p = line.c_str();
    unsigned char a = 0;
    unsigned char b = 0;
    while (p && *p) {
        b = *p;
        if (a != 0) {
            double trans = get(accepted_char(a), accepted_char(b));
            //printf("trans %c -> %c: %f\n", a, b, trans);
            log_prob += trans;
            transition_ct += 1;
        }
        p++;
        a = b;
    }
    return exp(log_prob / ((transition_ct > 1) ? transition_ct : 1));
}

// converts a letter like 'a' into the respective index in the string
// of accepted characters -- or 0 if the letter isn't in the string
uint8_t ProbabilityMatrix::accepted_char(uint8_t c) const
{
    return m_accepted_charmap[c];
}

void ProbabilityMatrix::sub(uint32_t x, uint32_t y, double value)
{
    if (!m_data) {
        return;
    }
    m_data[(ROW_SIZE * y) + x] -= value;
}

void ProbabilityMatrix::add(uint32_t x, uint32_t y, double value)
{
    if (!m_data) {
        return;
    }
    m_data[(ROW_SIZE * y) + x] += value;
}

void ProbabilityMatrix::set(uint32_t x, uint32_t y, double value)
{
    if (!m_data) {
        return;
    }
    m_data[(ROW_SIZE * y) + x] = value;
}

double ProbabilityMatrix::get(uint32_t x, uint32_t y) const
{
    if (!m_data) {
        return 0;
    }
    return m_data[(ROW_SIZE * y) + x];
}

bool ProbabilityMatrix::normalize()
{
    //          Normalize the counts so that they become log probabilities.
    //          We use log probabilities rather than straight probabilities to avoid
    //          numeric underflow issues with long texts.
    //          This contains a justification:
    //          http://squarecog.wordpress.com/2009/01/10/dealing-with-underflow-in-joint-probability-calculations/
    double sum = 0.0;
    size_t i = 0;
    size_t j = 0;
    for (i = 0; i < ROW_SIZE; ++i) {
        sum = 0.0;
        for (j = 0; j < ROW_SIZE; ++j) {
            double dprobmat = get(i, j);
            sum += dprobmat;
            if (dprobmat != 0.0) {
                //printf("Sum += %.8f\n", dprobmat);
            }
        }
        //printf("j / sum: %f     log(j / sum): %f\n", j / sum, log(j / sum));
        for (j = 0; j < ROW_SIZE; ++j) {
            set(i, j, log(j / sum));
        }
    }
    return true;
}
