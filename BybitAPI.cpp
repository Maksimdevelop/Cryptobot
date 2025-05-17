#include <msclr\marshal_cppstd.h>
#include "MainForm.h"
#include "BybitAPI.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace System;
using namespace System::Globalization;
using namespace System::IO;
using namespace System::Drawing;
using namespace msclr::interop;
using namespace System::Collections::Generic;
using namespace System::Security::Cryptography;
using namespace System::Net;

Cryptobot::BybitAPI::BybitAPI(System::String^ apiKey, System::String^ secretKey, System::String^ apiUrl) :
    apiKey_(apiKey),
    secretKey_(secretKey),
    apiUrl_(apiUrl)
{
    baseURL_ = "https://api.bybit.com";
    apiSecret_ = secretKey;
}

System::String^ Cryptobot::BybitAPI::GetPrice(System::String^ symbol) {
    if (String::IsNullOrEmpty(symbol)) {
        return "Error: Symbol is empty";
    }

    WebClient^ client = nullptr;
    try {
        client = gcnew WebClient();

        String^ category;
        if (symbol->Contains("-")) {
            category = "option";
        }
        else if (symbol->Contains("USDT")) {
            category = "linear";
        }
        else if (symbol->Contains("USD")) {
            category = "inverse";
        }
        else {
            category = "spot";
        }

        String^ url = String::Format(
            "{0}/v5/market/tickers?category={1}&symbol={2}",
            apiUrl_, category, Uri::EscapeDataString(symbol)
        );

        System::Diagnostics::Debug::WriteLine("GetPrice: URL=" + url);
        MainForm::logPanel_->Log("GetPrice: URL=" + url, Color::DarkGray);

        String^ response = client->DownloadString(url);

        String^ jsonString = JsonHelperDotNet::Json::ParseJson(response);

        if (String::IsNullOrEmpty(jsonString)) {
            System::Diagnostics::Debug::WriteLine("GetPrice: Error - Could not parse JSON response");
            MainForm::logPanel_->Log("GetPrice: Error - Could not parse JSON response", Color::Red);
            return "Error: Could not parse JSON response";
        }

        String^ retCodeStr = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retCode");
        if (String::IsNullOrEmpty(retCodeStr)) {
            System::Diagnostics::Debug::WriteLine("GetPrice: Error - retCode not found");
            MainForm::logPanel_->Log("GetPrice: Error - retCode not found", Color::Red);
            return "Error: retCode not found";
        }

        int retCode = Convert::ToInt32(retCodeStr);
        if (retCode != 0) {
            String^ retMsg = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retMsg");
            System::Diagnostics::Debug::WriteLine("GetPrice: Error - API returned " + retCode.ToString() + ", " + retMsg);
            MainForm::logPanel_->Log("GetPrice: Error - API returned " + retCode.ToString() + ", " + retMsg, Color::Red);
            return "Error: API returned " + retCode.ToString() + ", " + retMsg;
        }

        String^ resultString = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "result");
        String^ listString = JsonHelperDotNet::Json::GetPropertyValue(resultString, "list");

        if (String::IsNullOrEmpty(listString)) {
            System::Diagnostics::Debug::WriteLine("GetPrice: Error - No data for symbol " + symbol + " in category " + category);
            MainForm::logPanel_->Log("GetPrice: Error - No data for symbol " + symbol + " in category " + category, Color::Red);
            return "Error: No data for symbol " + symbol + " in category " + category;
        }

        String^ tickerString = JsonHelperDotNet::Json::GetArrayElement(listString, 0);
        if (String::IsNullOrEmpty(tickerString)) {
            System::Diagnostics::Debug::WriteLine("GetPrice: Error - No ticker data");
            MainForm::logPanel_->Log("GetPrice: Error - No ticker data", Color::Red);
            return "Error: No ticker data";
        }

        String^ lastPrice = JsonHelperDotNet::Json::GetPropertyValue(tickerString, "lastPrice");
        if (String::IsNullOrEmpty(lastPrice)) {
            System::Diagnostics::Debug::WriteLine("GetPrice: Error - lastPrice not found");
            MainForm::logPanel_->Log("GetPrice: Error - lastPrice not found", Color::Red);
            return "Error: lastPrice not found";
        }

        System::Diagnostics::Debug::WriteLine("GetPrice: lastPrice=" + lastPrice);
        MainForm::logPanel_->Log("GetPrice: lastPrice=" + lastPrice, Color::Green);

        return lastPrice;
    }
    catch (WebException^ e) {
        System::Diagnostics::Debug::WriteLine("GetPrice: Network Error - " + e->Message);
        MainForm::logPanel_->Log("GetPrice: Network Error - " + e->Message, Color::Red);
        return "Network Error: " + e->Message;
    }
    catch (Exception^ e) {
        System::Diagnostics::Debug::WriteLine("GetPrice: General Error - " + e->Message);
        MainForm::logPanel_->Log("GetPrice: General Error - " + e->Message, Color::Red);
        return "General Error: " + e->Message;
    }
    finally {
        if (client != nullptr) {
            delete client;
        }
    }
}

String^ Cryptobot::BybitAPI::PlaceOrder(String^ symbol, String^ side, double quantity, double price) {
    String^ endpoint = "/v5/order/create";
    String^ method = "POST";
    String^ timestamp = Convert::ToString((long long)(DateTime::UtcNow - DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind::Utc)).TotalMilliseconds);
    String^ recvWindow = "60000";

    String^ category;
    if (symbol->Contains("-")) {
        category = "option"; 
    }
    else if (symbol->Contains("USDT")) {
        category = "linear"; 
    }
    else if (symbol->Contains("USD")) {
        category = "inverse"; 
    }
    else {
        category = "spot"; 
    }

    String^ qtyStr = quantity.ToString("F2", CultureInfo::InvariantCulture);
    String^ priceStr = price.ToString("F2", CultureInfo::InvariantCulture);

    MainForm::logPanel_->Log("PlaceOrder: symbol=" + symbol + ", side=" + side + ", quantity=" + qtyStr + ", price=" + priceStr, Color::Green);

    String^ requestBody = String::Format(
        "{{"
        "\"category\":\"{0}\","
        "\"symbol\":\"{1}\","
        "\"side\":\"{2}\","
        "\"order_type\":\"Limit\","  
        "\"qty\":\"{3}\","
        "\"price\":\"{4}\","
        "\"time_in_force\":\"GTC\""   
        "}}",
        category, symbol, side, qtyStr, priceStr
    );

    String^ signString = timestamp + apiKey_ + recvWindow + requestBody;

    String^ sign = GenerateSignature(signString, apiSecret_);

    WebClient^ client = nullptr;
    try {
        client = gcnew WebClient();

        client->Headers->Add("Content-Type", "application/json");
        client->Headers->Add("X-BAPI-API-KEY", apiKey_);
        client->Headers->Add("X-BAPI-SIGN", sign);
        client->Headers->Add("X-BAPI-SIGN-TYPE", "SHA256");
        client->Headers->Add("X-BAPI-TIMESTAMP", timestamp);
        client->Headers->Add("X-BAPI-RECV-WINDOW", recvWindow);

        String^ url = apiUrl_ + endpoint;
        String^ response = client->UploadString(url, method, requestBody);

        String^ jsonString = JsonHelperDotNet::Json::ParseJson(response);
        if (String::IsNullOrEmpty(jsonString)) {
            return "Error: Could not parse JSON response";
        }

        String^ retCodeStr = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retCode");
        if (String::IsNullOrEmpty(retCodeStr) || Convert::ToInt32(retCodeStr) != 0) {
            String^ retMsg = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retMsg");
            return "Error: API returned " + retCodeStr + ", " + retMsg;
        }

        return "Order placed successfully";
    }
    catch (WebException^ ex) {
        return "WebException: " + ex->Message;
    }
    catch (Exception^ ex) {
        return "Exception: " + ex->Message;
    }
    finally {
        if (client != nullptr) {
            delete client;
        }
    }
}


using namespace System::Net;
using namespace System::Collections::Generic;

List<double>^ Cryptobot::BybitAPI::GetHistoricalData(System::String^ symbol, long long startTime, long long endTime, int interval) {
    if (String::IsNullOrEmpty(symbol)) {
        MainForm::logPanel_->Log("Error: Symbol is empty", Color::Red);
        System::Diagnostics::Debug::WriteLine("Error: Symbol is empty");
        return nullptr;
    }
    if (startTime >= endTime) {
        MainForm::logPanel_->Log("Error: startTime must be less than endTime", Color::Red);
        System::Diagnostics::Debug::WriteLine("Error: startTime must be less than endTime");
        return nullptr;
    }

    List<double>^ prices = gcnew List<double>();
    WebClient^ client = nullptr;
    try {
        client = gcnew WebClient();

        String^ category;
        if (symbol->Contains("-")) {
            category = "option";
        }
        else if (symbol->Contains("USDT")) {
            category = "linear";
        }
        else if (symbol->Contains("USD")) {
            category = "inverse";
        }
        else {
            category = "spot";
        }

        String^ endpoint = String::Format(
            "{0}/v5/market/kline?category={1}&symbol={2}&interval={3}&start={4}&end={5}&limit=1000",
            baseURL_, category, Uri::EscapeDataString(symbol), interval, startTime, endTime
        );

        System::Diagnostics::Debug::WriteLine("Request URL: " + endpoint);

        String^ response = client->DownloadString(endpoint);

        System::Diagnostics::Debug::WriteLine("API Response: " + response);

        String^ jsonString = JsonHelperDotNet::Json::ParseJson(response);
        if (String::IsNullOrEmpty(jsonString)) {
            MainForm::logPanel_->Log("Error: Could not parse JSON response", Color::Red);
            System::Diagnostics::Debug::WriteLine("Error: Could not parse JSON response");
            return nullptr;
        }

        String^ retCodeStr = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retCode");
        if (String::IsNullOrEmpty(retCodeStr)) {
            MainForm::logPanel_->Log("Error: retCode not found", Color::Red);
            System::Diagnostics::Debug::WriteLine("Error: retCode not found");
            return nullptr;
        }

        int retCode = Convert::ToInt32(retCodeStr);
        if (retCode != 0) {
            String^ retMsg = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retMsg");
            MainForm::logPanel_->Log("Error code: " + retCode.ToString() + ", Message: " + retMsg, Color::Red);
            System::Diagnostics::Debug::WriteLine("Error code: " + retCode.ToString() + ", Message: " + retMsg);
            return nullptr;
        }

        String^ resultString = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "result");
        if (String::IsNullOrEmpty(resultString)) {
            MainForm::logPanel_->Log("Error: result not found", Color::Red);
            System::Diagnostics::Debug::WriteLine("Error: result not found");
            return nullptr;
        }

        String^ listString = JsonHelperDotNet::Json::GetPropertyValue(resultString, "list");
        if (String::IsNullOrEmpty(listString) || listString == "[]") {
            MainForm::logPanel_->Log("Warning: Empty kline list", Color::Yellow);
            System::Diagnostics::Debug::WriteLine("Warning: Empty kline list");
            return prices;
        }

        int klineIndex = 0;
        String^ klineString = JsonHelperDotNet::Json::GetArrayElement(listString, klineIndex);
        while (!String::IsNullOrEmpty(klineString)) {
            String^ closePrice = JsonHelperDotNet::Json::GetArrayElement(klineString, 4);
            if (!String::IsNullOrEmpty(closePrice)) {
                try {
                    double parsedPrice;
                    if (Double::TryParse(closePrice, NumberStyles::Any, CultureInfo::InvariantCulture, parsedPrice)) {
                        prices->Add(parsedPrice);
                    }
                    else {
                        MainForm::logPanel_->Log("Error: Invalid number format in close price: " + closePrice, Color::Red);
                        System::Diagnostics::Debug::WriteLine("Error: Invalid number format in close price: " + closePrice);
                    }

                }
                catch (FormatException^) {
                    MainForm::logPanel_->Log("Error: Invalid number format in close price: " + closePrice, Color::Red);
                    System::Diagnostics::Debug::WriteLine("Error: Invalid number format in close price: " + closePrice);
                }
            }
            else {
                MainForm::logPanel_->Log("Error: closePrice not found at kline index " + klineIndex, Color::Red);
                System::Diagnostics::Debug::WriteLine("Error: closePrice not found at kline index " + klineIndex);
            }

            klineIndex++;
            klineString = JsonHelperDotNet::Json::GetArrayElement(listString, klineIndex);
        }
    }
    catch (WebException^ e) {
        MainForm::logPanel_->Log("Network Error: " + e->Message, Color::Red);
        System::Diagnostics::Debug::WriteLine("Network Error: " + e->Message);
        return nullptr;
    }
    catch (Exception^ e) {
        MainForm::logPanel_->Log("General Error: " + e->Message, Color::Red);
        System::Diagnostics::Debug::WriteLine("General Error: " + e->Message);
        return nullptr;
    }
    finally {
        if (client != nullptr) {
            delete client;
        }
    }
    return prices;
}

System::String^ Cryptobot::BybitAPI::GetBalance(System::String^ currency) {
    WebClient^ client = gcnew WebClient();
    System::String^ ret = "Error";
    try {
        System::String^ url = apiUrl_ + "/v5/account/wallet-balance?accountType=CONTRACT&coin=" + Uri::EscapeDataString(currency);

        auto timestamp = static_cast<long long>(DateTime::Now.Subtract(DateTime(1970, 1, 1)).TotalMilliseconds);
        System::String^ timestampStr = timestamp.ToString();

        System::String^ queryString = "accountType=CONTRACT&coin=" + currency;
        System::String^ paramStr = timestampStr + apiKey_ + queryString;

        array<Byte>^ secretBytes = System::Text::Encoding::UTF8->GetBytes(apiSecret_);
        array<Byte>^ paramBytes = System::Text::Encoding::UTF8->GetBytes(paramStr); 
        HMACSHA256^ hmac = gcnew HMACSHA256(secretBytes);
        array<Byte>^ hash = hmac->ComputeHash(paramBytes); 
        String^ signature = BitConverter::ToString(hash)->Replace("-", "")->ToLower();

        client->Headers->Add("X-Bapi-Api-Key", apiKey_);
        client->Headers->Add("X-Bapi-Sign", signature);
        client->Headers->Add("X-Bapi-Timestamp", timestampStr);
        client->Headers->Add("Content-Type", "application/json");

        System::String^ response = client->DownloadString(url);

        String^ jsonString = JsonHelperDotNet::Json::ParseJson(response);
          if (String::IsNullOrEmpty(jsonString))
        {
            ret = "Error: Could not parse JSON response";
            return ret;
        }

        String^ retCodeStr = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retCode");
        if (String::IsNullOrEmpty(retCodeStr)) {
            ret = "Error: retCode not found";
            return ret;
        }

        int retCode = Convert::ToInt32(retCodeStr);
        if (retCode == 0) {
            String^ resultString = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "result");
            if (String::IsNullOrEmpty(resultString)) {
                ret = "Error: result not found";
                return ret;
            }

            String^ listString = JsonHelperDotNet::Json::GetPropertyValue(resultString, "list");
            if (String::IsNullOrEmpty(listString)) {
                ret = "Error: list not found";
                return ret;
            }

            String^ firstElement = JsonHelperDotNet::Json::GetArrayElement(listString, 0);
            if (String::IsNullOrEmpty(firstElement)) {
                ret = "Error: first element in list not found";
                return ret;
            }

            String^ coinString = JsonHelperDotNet::Json::GetPropertyValue(firstElement, "coin");
             if (String::IsNullOrEmpty(coinString)) {
                ret = "Error: coin not found";
                return ret;
            }

           String^ firstCoinElement = JsonHelperDotNet::Json::GetArrayElement(coinString, 0);
            if (String::IsNullOrEmpty(firstCoinElement)) {
                ret = "Error: first coin element not found";
                return ret;
            }

            ret = JsonHelperDotNet::Json::GetPropertyValue(firstCoinElement, "walletBalance");
            if (String::IsNullOrEmpty(ret)) {
                ret = "Error: walletBalance not found";
            }
        }
        else {
            String^ retMsg = JsonHelperDotNet::Json::GetPropertyValue(jsonString, "retMsg");
            ret = "Error code: " + retCode.ToString() + ", Message: " + retMsg;
        }
    }
    catch (WebException^ e) {
        ret = "Network Error: " + e->Message;
    }
    catch (Exception^ e) {
        ret = "General Error: " + e->Message;
    }
    finally {
        delete client;
    }
    return ret;
}

String^ Cryptobot::BybitAPI::GenerateSignature(String^ message, String^ secret) {
    array<unsigned char>^ secretBytes = System::Text::Encoding::UTF8->GetBytes(secret);
    array<unsigned char>^ messageBytes = System::Text::Encoding::UTF8->GetBytes(message);

    HMACSHA256^ hmac = gcnew HMACSHA256(secretBytes);
    array<unsigned char>^ hashBytes = hmac->ComputeHash(messageBytes);

    String^ hexString = "";
    for (int i = 0; i < hashBytes->Length; i++) {
        hexString += hashBytes[i].ToString("x2");
    }

    return hexString;
}
