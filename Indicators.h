#pragma once

#include <vector>

namespace Cryptobot {
    class Indicators {
    public:
        static double CalculateSMA(const std::vector<double>& data, int period);
        static double CalculateRSI(const std::vector<double>& data, int period);
    };
}