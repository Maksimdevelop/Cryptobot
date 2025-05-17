#include "MainForm.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "OrderInfo.h"
#include "LogPanel.h"

using namespace System;
using namespace System::Globalization;
using namespace System::Windows::Forms;
using namespace Cryptobot;
using namespace System::Threading;
using namespace System::IO;
using namespace System::Drawing;

[STAThreadAttribute]
int Main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    Application::Run(gcnew MainForm());

    return 0;
}

MainForm::MainForm() {
    InitializeComponent();
    bybitApi_ = gcnew BybitAPI("API", "SECRET", "https://api-testnet.bybit.com");
    strategy_ = new Strategy(10, 30, 14);
    orders_ = gcnew System::Collections::Generic::List<OrderInfo^>();

    logPanel_->Log("Application started", Color::Green);
}

MainForm::~MainForm() {
    if (components) {
        delete components;
    }
    if (strategy_ != nullptr) {
        delete strategy_;
    }
}

MainForm::!MainForm() {
    if (components) {
        delete components;
    }
    if (strategy_ != nullptr) {
        delete strategy_;
    }
}

void MainForm::getPriceButtonClick(System::Object^ sender, System::EventArgs^ e) {
    String^ symbol = "BTCUSDT";
    String^ priceString = bybitApi_->GetPrice(symbol);

    if (priceString->StartsWith("Error")) {
        logPanel_->Log("Error getting price: " + priceString, Color::Red);
        MessageBox::Show(priceString, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }
    else {
        logPanel_->Log("Price for " + symbol + ": " + priceString, Color::Blue);
        priceTextBox->Text = priceString;
    }
}

void MainForm::StartStopButtonClick(System::Object^ sender, System::EventArgs^ e) {
    if (!tradingEnabled_) {
        tradingEnabled_ = true;
        stopTrading_ = false;
        startStopButton->Text = L"Stop";
        tradeThread_ = gcnew Thread(gcnew ThreadStart(this, &MainForm::TradeLoop));
        tradeThread_->Start();
    }
    else {
        stopTrading_ = true;
        tradingEnabled_ = false;
        startStopButton->Text = L"Start";
        if (tradeThread_ != nullptr && tradeThread_->IsAlive) {
            if (!tradeThread_->Join(500))
                tradeThread_->Abort();
        }
    }

    if (!tradingEnabled_) {;
    }
}

void MainForm::TradeLoop() {
    System::String^ symbol = "BTCUSDT";
    long long startTime = (long long)(DateTime::Now.AddDays(-2).ToUniversalTime() - DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind::Utc)).TotalSeconds * 1000; // Start time 2 days ago (in milliseconds)
    int interval = 15;

    while (!stopTrading_) {
        try {
            System::Collections::Generic::List<double>^ historicalPricesNet = bybitApi_->GetHistoricalData(symbol, startTime, (long long)(DateTime::Now.ToUniversalTime() - DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind::Utc)).TotalSeconds * 1000, interval);

            if (historicalPricesNet != nullptr) {
                std::vector<double> historicalPrices;
                for (int i = 0; i < historicalPricesNet->Count; ++i) {
                    historicalPrices.push_back(System::Convert::ToDouble(historicalPricesNet[i]));
                }

                Strategy::Signal signal = strategy_->GetSignal(historicalPrices);

                String^ currentPriceString = bybitApi_->GetPrice(symbol);

                if (currentPriceString->StartsWith("Error")) {
                    logPanel_->Log("Error getting current price: " + currentPriceString, Color::Red);
                    Thread::Sleep(5000);
                    continue;
                }

                double currentPrice;
                if (Double::TryParse(currentPriceString, NumberStyles::Any, Globalization::CultureInfo::InvariantCulture, currentPrice)) {
                    logPanel_->Log("TradeLoop: currentPrice = " + currentPrice.ToString(), Color::Green);
                }
                else {
                    logPanel_->Log("TradeLoop: Error parsing currentPriceString: " + currentPriceString, Color::Red);
                    continue;
                }

                String^ orderResult;
                String^ side;
                switch (signal) {
                case Strategy::Signal::BUY:
                    side = "Buy";
                    logPanel_->Log("Buy signal: " + currentPrice.ToString());
                    orderResult = bybitApi_->PlaceOrder(symbol, side, 0.01, currentPrice);
                    logPanel_->Log("Buy order result: " + orderResult);
                    break;
                case Strategy::Signal::SELL:
                    side = "Sell";
                    logPanel_->Log("Sell signal: " + currentPrice.ToString());
                    orderResult = bybitApi_->PlaceOrder(symbol, side, 0.01, currentPrice);
                    logPanel_->Log("Sell order result: " + orderResult);
                    break;
                case Strategy::Signal::HOLD:
                    logPanel_->Log("Hold signal: " + currentPrice.ToString());
                    continue;
                default:
                    continue;
                }

                OrderInfo^ order = gcnew OrderInfo(symbol, side, currentPrice, 0.01);
                orders_->Add(order);

                String^ logFilePath = "orders.txt";
                StreamWriter^ sw = nullptr;
                try {
                    sw = gcnew StreamWriter(logFilePath, true);
                    sw->WriteLine(order->ToString());
                }
                finally {
                    if (sw != nullptr)
                        sw->Close();
                }
            }
            else {
                logPanel_->Log("Error: historicalPricesNet is null.", Color::Red);
            }
        }
        catch (System::Exception^ ex) {
            System::Diagnostics::Debug::WriteLine("Exception in TradeLoop: " + ex->Message, Color::Red);
        }
        finally {
            Thread::Sleep(5000);
        }
    }
}

void MainForm::CalculateAndShowResults() {
    double totalProfit = 0.0;
    double initialBalance = 10.0;

    String^ initialBalanceString = bybitApi_->GetBalance("USDT");
    if (initialBalanceString->StartsWith("Error")) {
        logPanel_->Log("Error getting initial balance: " + initialBalanceString, Color::Red);
        return;
    }

    try {
        initialBalance = System::Double::Parse(initialBalanceString);
    }
    catch (System::Exception^ ex) {
        logPanel_->Log("Error parsing initial balance: " + ex->Message, Color::Red);
        return;
    }

    String^ currentBalanceString = bybitApi_->GetBalance("USDT");
    if (currentBalanceString->StartsWith("Error")) {
        logPanel_->Log("Error getting current balance: " + currentBalanceString, Color::Red);
        return;
    }

    double currentBalance = 0.0;
    try {
        currentBalance = System::Double::Parse(currentBalanceString);
    }
    catch (System::Exception^ ex) {
        logPanel_->Log("Error parsing current balance: " + ex->Message, Color::Red);
        return;
    }

    totalProfit = currentBalance - initialBalance;

    String^ message = "Initial Balance: " + initialBalance.ToString() + "\nCurrent Balance: " + currentBalance.ToString() + "\nTotal Profit/Loss: " + totalProfit.ToString();
    MessageBox::Show(message, "Trading Results", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

void MainForm::InitializeComponent() {
    if (this->components == nullptr) {
        this->components = gcnew System::ComponentModel::Container();
    }
    this->getPriceButton = gcnew System::Windows::Forms::Button();
    this->priceTextBox = gcnew System::Windows::Forms::TextBox();
    this->startStopButton = gcnew System::Windows::Forms::Button();
    this->logPanel_ = (gcnew ::Cryptobot::LogPanel());
    this->SuspendLayout();

    this->apiKeyFileLabel = (gcnew System::Windows::Forms::Label());
    this->apiKeyFileLabel->AutoSize = true;
    this->apiKeyFileLabel->Location = System::Drawing::Point(24, 120);
    this->apiKeyFileLabel->Name = L"apiKeyFileLabel";
    this->apiKeyFileLabel->Size = System::Drawing::Size(90, 13);
    this->apiKeyFileLabel->TabIndex = 4;
    this->apiKeyFileLabel->Text = L"API Key File:";

    this->apiKeyFilePathTextBox = (gcnew System::Windows::Forms::TextBox());
    this->apiKeyFilePathTextBox->Location = System::Drawing::Point(24, 140);
    this->apiKeyFilePathTextBox->Name = L"apiKeyFilePathTextBox";
    this->apiKeyFilePathTextBox->Size = System::Drawing::Size(100, 20);
    this->apiKeyFilePathTextBox->TabIndex = 5;

    this->apiKeyFileBrowseButton = (gcnew System::Windows::Forms::Button());
    this->apiKeyFileBrowseButton->Location = System::Drawing::Point(24, 160);
    this->apiKeyFileBrowseButton->Name = L"apiKeyFileBrowseButton";
    this->apiKeyFileBrowseButton->Size = System::Drawing::Size(75, 23);
    this->apiKeyFileBrowseButton->TabIndex = 6;
    this->apiKeyFileBrowseButton->Text = L"Browse...";
    this->apiKeyFileBrowseButton->UseVisualStyleBackColor = true;
    this->apiKeyFileBrowseButton->Click += gcnew System::EventHandler(this, &MainForm::ApiKeyFileBrowseButton_Click);

    this->apiKeyOpenFileDialog = (gcnew System::Windows::Forms::OpenFileDialog());
    this->apiKeyOpenFileDialog->Filter = L"Config files (*.cfg)|*.cfg|All files (*.*)|*.*";
    this->apiKeyOpenFileDialog->Title = L"Select API Key File";

    this->getPriceButton->Location = System::Drawing::Point(24, 22);
    this->getPriceButton->Name = L"getPriceButton";
    this->getPriceButton->Size = System::Drawing::Size(75, 23);
    this->getPriceButton->TabIndex = 0;
    this->getPriceButton->Text = L"Get Price";
    this->getPriceButton->UseVisualStyleBackColor = true;
    this->getPriceButton->Click += gcnew System::EventHandler(this, &MainForm::getPriceButtonClick);

    this->priceTextBox->Location = System::Drawing::Point(24, 47);
    this->priceTextBox->Name = L"priceTextBox";
    this->priceTextBox->Size = System::Drawing::Size(100, 20);
    this->priceTextBox->TabIndex = 1;

    this->startStopButton->Location = System::Drawing::Point(24, 82);
    this->startStopButton->Name = L"startStopButton";
    this->startStopButton->Size = System::Drawing::Size(75, 23);
    this->startStopButton->TabIndex = 2;
    this->startStopButton->Text = L"Start";
    this->startStopButton->UseVisualStyleBackColor = true;
    this->startStopButton->Click += gcnew System::EventHandler(this, &MainForm::StartStopButtonClick);

    this->logPanel_->Anchor = System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right;
    this->logPanel_->Location = System::Drawing::Point(150, 22);
    this->logPanel_->Name = L"logPanel_";
    this->logPanel_->Size = System::Drawing::Size(340, 200);
    this->logPanel_->TabIndex = 3;

    this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
    this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
    this->ClientSize = System::Drawing::Size(500, 250);
    this->Controls->Add(this->logPanel_);
    this->Controls->Add(this->startStopButton);
    this->Controls->Add(this->priceTextBox);
    this->Controls->Add(this->getPriceButton);
    this->Controls->Add(this->apiKeyFileLabel);
    this->Controls->Add(this->apiKeyFilePathTextBox);
    this->Controls->Add(this->apiKeyFileBrowseButton);
    this->Name = L"MainForm";
    this->Text = L"Cryptobot";
    this->ResumeLayout(false);
    this->PerformLayout();

}

void Cryptobot::MainForm::GetPriceThread(Object^ symbol)
{
    try
    {
        String^ price = bybitApi_->GetPrice((String^)symbol);
        if (price->StartsWith("Error"))
        {
            MessageBox::Show(price, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
        else
        {
            priceTextBox->Text = price;
        }
    }
    catch (Exception^ ex)
    {
        MessageBox::Show(ex->Message, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }
}

void MainForm::ApiKeyFileBrowseButton_Click(System::Object^ sender, System::EventArgs^ e) {
    if (apiKeyOpenFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
        apiKeyFilePathTextBox->Text = apiKeyOpenFileDialog->FileName;
        LoadApiKeysFromFile(apiKeyOpenFileDialog->FileName);
    }
}

void MainForm::LoadApiKeysFromFile(String^ filePath) {
    try {
        StreamReader^ sr = gcnew StreamReader(filePath);
        String^ line;
        while ((line = sr->ReadLine()) != nullptr) {
            if (line->StartsWith("apiKey=")) {
                bybitApi_->apiKey_ = line->Substring(7)->Trim();
            }
            else if (line->StartsWith("apiSecret=")) {
                bybitApi_->apiSecret_ = line->Substring(10)->Trim();
            }
        }
        sr->Close();
        logPanel_->Log("API Keys loaded from file: " + filePath, Color::Green);
    }
    catch (Exception^ ex) {
        logPanel_->Log("Error loading API Keys from file: " + ex->Message, Color::Red);
        MessageBox::Show("Error loading API Keys from file: " + ex->Message, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }
}