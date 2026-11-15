#pragma once

#include "core/ScanResult.hpp"

#include <string>
#include <vector>

class DetectionRule {
public:
    virtual ~DetectionRule() = default;
    virtual std::string name() const = 0;
    virtual std::vector<std::string> evaluate(const ScanResult& result) const = 0;
};
