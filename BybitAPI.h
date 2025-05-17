#pragma once

#include <string>
#include <vector>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Text;

namespace Cryptobot {

    public ref class BybitAPI
    {
    public:
        System::String^ apiKey_;
        System::String^ secretKey_;
        System::String^ apiSecret_;
        BybitAPI(System::String^ apiKey, System::String^ secretKey, System::String^ apiUrl);
        System::String^ GetPrice(System::String^ symbol);
        System::String^ PlaceOrder(System::String^ symbol, System::String^ side, double quantity, double price);
        System::Collections::Generic::List<double>^ GetHistoricalData(System::String^ symbol, long long startTime, long long endTime, int interval);
        System::String^ GetBalance(System::String^ currency);
        System::String^ GenerateSignature(System::String^ data, System::String^ secretKey);

    private:
    private:
        System::String^ apiUrl_;
        System::String^ baseURL_;
    };
}