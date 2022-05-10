#pragma once

#include <string>
#include <vector>

class ProbabilityMatrix
{
public:
    // create new probability matrix with a string of letters to train for
    ProbabilityMatrix(const std::string &accepted_chars);

    // train the probability matrix on a list of strings (good or bad)
    void train(const std::vector<std::string> &data, bool good);

    // calculate the average transition probability for a line, the transition
    // probability is how likely any given letter is to transition to the next
    double avg_trans(const std::string &line) const;

private:
    // converts a letter like 'a' into the respective index in the string
    // of accepted characters -- or 0 if the letter isn't in the string
    uint8_t accepted_char(uint8_t c) const;

    // accessors to the matrix
    void sub(uint32_t x, uint32_t y, double value);
    void add(uint32_t x, uint32_t y, double value);
    void set(uint32_t x, uint32_t y, double value);
    double get(uint32_t x, uint32_t y) const;

    // normalize the matrix
    bool normalize();

    // load a training file
    std::vector<std::string> load_file(const std::string &filename);

    // normalize training data so that only the supported characters are trained
    std::string normalize_string(const std::string &line);

private:
    // a string of acceptable letters
    std::string m_accepted_chars;
    // the size of a row
    size_t m_rowsize;
    // the size of the full matrix
    size_t m_fullsize;
    // a map for all ascii letters, where the index is a letter and
    // the value stored there is an index in the m_accepted_chars string
    // this is because the m_accepted_chars string defines the width of
    // the data matrix via it's string length. The accepted_charmap allows
    // for O(1) conversion from a letter into an index in m_accepted_chars
    // which is equivalent to an x or y position in the matrix
    uint8_t m_accepted_charmap[256];
    // the data matrix
    double *m_data;

};


