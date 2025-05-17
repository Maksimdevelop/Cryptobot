#include "Indicators.h"
#include <numeric>
#include <cmath>
#include "MainForm.h"

using namespace System::Drawing;

namespace Cryptobot {
    double Indicators::CalculateSMA(const std::vector<double>& data, int period) {
        if (data.size() < period) {
            MainForm::logPanel_->Log("Indicators::CalculateSMA - Not enough data", Color::Red);
            return 0.0;
        }

        double sum = std::accumulate(data.end() - period, data.end(), 0.0);
        MainForm::logPanel_->Log("Indicators::CalculateSMA - SMA : " + (sum / period).ToString(), Color::Green);
        return sum / period;
    }

    double Indicators::CalculateRSI(const std::vector<double>& data, int period) {
        if (data.size() <= period) {
            MainForm::logPanel_->Log("Indicators::CalculateRSI - Not enough data", Color::Red);
            return 50.0;
        }

        double avgGain = 0.0;
        double avgLoss = 0.0;

        for (size_t i = data.size() - period - 1; i < data.size() - 1; ++i) {
            double change = data[i + 1] - data[i];
            if (change > 0) {
                avgGain += change;
            }
            else {
                avgLoss += std::abs(change);
            }
        }

        avgGain /= period;
        avgLoss /= period;

        if (avgLoss == 0) {
            MainForm::logPanel_->Log("Indicators::CalculateRSI - avgLoss == 0, returning 100.0", Color::Green);
            return 100.0;
        }

        double rs = avgGain / avgLoss;
        double rsi = 100.0 - (100.0 / (1.0 + rs));
        MainForm::logPanel_->Log("Indicators::CalculateRSI - RSI : " + (rsi).ToString(), Color::Green);
        return rsi;
    }
}