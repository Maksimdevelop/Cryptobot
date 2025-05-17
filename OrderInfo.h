#pragma once

#include <string>

namespace Cryptobot {
    using namespace System;
    public ref class OrderInfo
    {
    public:
        DateTime Time;
        String^ Symbol;
        String^ Side;
        double Price;
        double Quantity;

        OrderInfo(String^ symbol, String^ side, double price, double quantity)
        {
            Time = DateTime::Now;
            Symbol = symbol;
            Side = side;
            Price = price;
            Quantity = quantity;
        }

        virtual String^ ToString() override
        {
            return Time.ToString() + " | " + Symbol + " | " + Side + " | Price: " + Price.ToString() + " | Quantity: " + Quantity.ToString();
        }
    };
}