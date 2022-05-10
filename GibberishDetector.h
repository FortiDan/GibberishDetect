#pragma once

#include <vector>
#include <string>

#include "ProbabilityMatrix.h"

class GibberishDetector
{
    static const std::string accepted_chars;

public:
    GibberishDetector();

    void init();
    
    bool is_gibberish(const std::string &str) const;

private:
    std::vector<std::string> load_file(const std::string &filename) const;
    std::string normalize_string(const std::string &line) const;

    // the probability matrix of character transitions
    ProbabilityMatrix m_probmat;
    // the threshold for average character transition being gibberish
    double m_threshold;
};


