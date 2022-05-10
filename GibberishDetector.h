#pragma once

#include <vector>
#include <string>

#include "ProbabilityMatrix.h"

class GibberishDetector
{
public:
    GibberishDetector();

    bool init(const std::string &accepted);
    
    bool is_gibberish(const std::string &str) const;

private:
    std::vector<std::string> load_file(const std::string &filename) const;
    std::string normalize_string(const std::string &line) const;

    // the probability matrix of character transitions
    ProbabilityMatrix m_probmat;
    // the threshold for average character transition being gibberish
    double m_threshold;

    // string of accepted characters
    std::string m_accepted_chars;

    // map of which characters are allowed, like an array_flip() of m_accepted_chars
    bool m_allowed_charmap[256];
};


