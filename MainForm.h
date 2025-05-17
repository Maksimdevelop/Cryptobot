#pragma once

#include <Windows.h>
#include "OrderInfo.h"
#include "LogPanel.h"
#include "Strategy.h"
#include "BybitAPI.h"

namespace Cryptobot {

    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm();
        ~MainForm();
        !MainForm();

    private:
        System::ComponentModel::Container^ components;
        System::Windows::Forms::Button^ getPriceButton;
        System::Windows::Forms::TextBox^ priceTextBox;
        System::Windows::Forms::Button^ startStopButton;

    private:
        BybitAPI^ bybitApi_;
        Strategy* strategy_;
        System::Collections::Generic::List<OrderInfo^>^ orders_;
        bool tradingEnabled_ = false;
        bool stopTrading_ = false;
        System::Threading::Thread^ tradeThread_;
    public:
        static LogPanel^ logPanel_;

    private:
        void getPriceButtonClick(System::Object^ sender, System::EventArgs^ e);
        void StartStopButtonClick(System::Object^ sender, System::EventArgs^ e);
        void TradeLoop();
        void CalculateAndShowResults();

        System::Windows::Forms::Label^ apiKeyFileLabel;
        System::Windows::Forms::TextBox^ apiKeyFilePathTextBox;
        System::Windows::Forms::Button^ apiKeyFileBrowseButton;
        System::Windows::Forms::OpenFileDialog^ apiKeyOpenFileDialog;
        void LoadApiKeysFromFile(String^ filePath);
        void ApiKeyFileBrowseButton_Click(System::Object^ sender, System::EventArgs^ e);

    protected:
        void InitializeComponent();
        void GetPriceThread(Object^ symbol);
    };
}