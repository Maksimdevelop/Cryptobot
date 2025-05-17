#include "Strategy.h"
#include "Indicators.h"
#include "MainForm.h"

using namespace System::Drawing;


namespace Cryptobot {
    Strategy::Strategy(int smaPeriodShort, int smaPeriodLong, int rsiPeriod)
        : smaPeriodShort_(smaPeriodShort), smaPeriodLong_(smaPeriodLong), rsiPeriod_(rsiPeriod) {}

    Strategy::Signal Strategy::GetSignal(const std::vector<double>& data) {
        if (data.size() < this->smaPeriodLong_ / 2) {
            MainForm::logPanel_->Log("Strategy::GetSignal: Not enough data (" + data.size().ToString() + " < " + this->smaPeriodLong_.ToString() + ")", Color::Yellow);
            return Signal::HOLD;
        }

        double smaShort = Indicators::CalculateSMA(data, smaPeriodShort_);
        double smaLong = Indicators::CalculateSMA(data, smaPeriodLong_);

        double rsi = Indicators::CalculateRSI(data, rsiPeriod_);

        System::Diagnostics::Debug::WriteLine("Strategy::GetSignal: smaShort=" + smaShort.ToString() + ", smaLong=" + smaLong.ToString() + ", rsi=" + rsi.ToString());
        if (smaShort > smaLong && rsi < 30) {
            MainForm::logPanel_->Log("Strategy::GetSignal: BUY signal", Color::Green);
            return Signal::BUY;
        }
        else if (smaShort < smaLong && rsi > 70) {
            MainForm::logPanel_->Log("Strategy::GetSignal: SELL signal", Color::Red);
            return Signal::SELL;
        }
        else {
            MainForm::logPanel_->Log("Strategy::GetSignal: HOLD signal", Color::Yellow);
            return Signal::HOLD;
        }
    }
}