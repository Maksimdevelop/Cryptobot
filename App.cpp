/*#include <msclr/marshal_cppstd.h>
#include "BybitAPI.h"
#include <iostream>

int main() {
    // Convert std::string to System::String
    System::String^ apiKey = gcnew System::String("ZV7Glkd3cuXf41cBxF");
    System::String^ secretKey = gcnew System::String("gQ24NN0ut027dQae9vDRpPo80wcj1Z1GdLGo");
    System::String^ baseURL = gcnew System::String("https://api-testnet.bybit.com");

    Cryptobot::BybitAPI^ bybitApi = gcnew Cryptobot::BybitAPI(apiKey, secretKey, baseURL);

    // Получаем цену
    System::String^ priceResult = bybitApi->GetPrice("BTCUSDT");
    std::cout << "Price Result: " << msclr::interop::marshal_as<std::string>(priceResult) << std::endl;
    system("pause");

    // Размещаем ордер
    //System::String^ orderResult = bybitApi->PlaceOrder("BTCUSDT", "Buy", 0.01, 20000.0);
    //std::cout << "Order Result: " << msclr::interop::marshal_as<std::string>(orderResult) << std::endl;

    // Получаем исторические данные
    //System::Collections::Generic::List<double>^ historicalData = bybitApi->GetHistoricalData("BTCUSDT", 1672531200, 1672534800, 60);
    //Console::WriteLine("Historical Data Count: " + historicalData->Count);

    return 0;
}*/