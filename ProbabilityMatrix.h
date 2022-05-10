#pragma once

#include <string>
#include <vector>

class ProbabilityMatrix
{
public:
    ProbabilityMatrix();

    // init probability matrix with a string of letters to train for
    bool init(const std::string &accepted_chars);

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

private:
    // a string of acceptable letters
    std::string m_accepted_chars;
    // a map of accepted chars to indexes in m_accepted_chars, like doing
    // array_flip in php
    uint8_t m_accepted_charmap[256];
    // the data matrix
    double *m_data;

};


