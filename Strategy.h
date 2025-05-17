#pragma once

#include <vector>
#include "Indicators.h"

namespace Cryptobot {
    class Strategy {
    public:
        enum class Signal {
            BUY,
            SELL,
            HOLD
        };

        Strategy(int smaPeriodShort, int smaPeriodLong, int rsiPeriod);

        Signal GetSignal(const std::vector<double>& data);

    private:
        int smaPeriodShort_;
        int smaPeriodLong_;
        int rsiPeriod_;
    };
}