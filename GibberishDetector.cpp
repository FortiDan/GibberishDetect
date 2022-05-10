#include "GibberishDetector.h"

#include <sstream>
#include <fstream>

using namespace std;

#define TRAININGDIR     "phrases/"
#define BIG_TEXT_FILE   TRAININGDIR "big.txt"
#define GOOD_TEXT_FILE  TRAININGDIR "good.txt"
#define BAD_TEXT_FILE   TRAININGDIR "bad.txt"

GibberishDetector::GibberishDetector() :
    m_probmat()
{
}

bool GibberishDetector::init(const std::string &accepted)
{
    m_accepted_chars = accepted;
    // import the accepted characters into an inverted map
    for (int i = 0; i < m_accepted_chars.length(); ++i) {
        // set each letter in accepted string as true in the charmap
        m_allowed_charmap[accepted[i]] = true;
    }
    // always accept newlines
    m_allowed_charmap['\n'] = true;

    // initialize the probability matrix
    if (!m_probmat.init(m_accepted_chars)) {
        return false;
    }

    // load the big text file full of training data
    vector<string> big_text = load_file(BIG_TEXT_FILE);
    if (!big_text.size()) {
        return false;
    }

    // train the probability matrix on the big text file
    m_probmat.train(big_text, true);

    // measure probabilities of all the "good lines"
    vector<string> good_text = load_file(GOOD_TEXT_FILE);
    double *good_probs = new double[good_text.size()];
    size_t i = 0;
    for (i = 0; i < good_text.size(); ++i) {
        good_probs[i] = m_probmat.avg_trans(good_text[i]);
        printf("[%s] - %f\n", good_text[i].c_str(), good_probs[i]);
    }

    // measure probabilities of all the "bad lines"
    vector<string> bad_text = load_file(BAD_TEXT_FILE);
    double *bad_probs = new double[bad_text.size()];
    for (i = 0; i < bad_text.size(); ++i) {
        bad_probs[i] = m_probmat.avg_trans(bad_text[i]);
        printf("[%s] - %f\n", bad_text[i].c_str(), bad_probs[i]);
    }

    // calculate the minimum good probability
    double min_good_probs = 1000000000;
    double max_bad_probs = 0.0;
    for (i = 0; i < good_text.size(); ++i) {
        if (min_good_probs > good_probs[i]) {
            min_good_probs = good_probs[i];
        }
    }
    // and the maximum bad probability
    for (i = 0; i < bad_text.size(); ++i) {
        if (max_bad_probs < bad_probs[i]) {
            max_bad_probs = bad_probs[i];
        }
    }

    // the minimum good should not be lower than the max bad
    if (min_good_probs <= max_bad_probs) {
        printf("minimum good [%f] smaller than max bad [%f]\n", min_good_probs, max_bad_probs);
    }

    // And pick a threshold halfway between the worst good and best bad inputs.
    m_threshold = (min_good_probs + max_bad_probs) / 2.0;
    printf("Threshold: %f\n", m_threshold);

    return true;
}

bool GibberishDetector::is_gibberish(const string &str) const
{
    double avg = m_probmat.avg_trans(normalize_string(str));
    printf("Average for [%s]: %f\n", str.c_str(), avg);
    return (avg < m_threshold);
}


vector<string> GibberishDetector::load_file(const string &filename) const
{
    vector<string> data_list;

    printf("Reading file [%s]...\n", filename.c_str());
    ifstream t(filename);
    stringstream buffer;
    buffer << t.rdbuf();
    string data = buffer.str();
    if (!data.length()) {
        return data_list;
    }

    printf("Normalizing data...\n");
    string norm_data = normalize_string(data);

    printf("Exploding by lines...\n");
    string::size_type pos = 0;
    string::size_type prev = 0;
    while ((pos = norm_data.find('\n', prev)) != string::npos) {
        string line = norm_data.substr(prev, pos - prev);
        if (line.length() > 0) {
            data_list.push_back(line);
        }
        prev = pos + 1;
    }
    // final line, usually empty
    string line = norm_data.substr(prev);
    if (line.length() > 0) {
        data_list.push_back(line);
    }

    return data_list;
}

string GibberishDetector::normalize_string(const string &line) const
{
    size_t len = 0;
    size_t i = 0;
    size_t j = 0;
    string copy;

    for (i = 0; i < line.size(); ++i) {
        // is this letter in accepted chars?
        char letter = tolower(line[i]);
        if (m_allowed_charmap[letter] == true) {
            // yep, copy it
            copy += string(1, tolower(line[i]));
        }
    }
    return copy;
}

