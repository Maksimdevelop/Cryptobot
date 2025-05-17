#include "LogPanel.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

Cryptobot::LogPanel::LogPanel() {
    logTextBox_ = gcnew RichTextBox();
    logTextBox_->Multiline = true;
    logTextBox_->ReadOnly = true;
    logTextBox_->ScrollBars = RichTextBoxScrollBars::Vertical;
    logTextBox_->Dock = DockStyle::Fill;
    this->Controls->Add(logTextBox_);

    this->Dock = DockStyle::Fill;
}

void Cryptobot::LogPanel::Log(System::String^ message) {
    AppendTextSafe(message + Environment::NewLine, Color::Black);
}

void Cryptobot::LogPanel::Log(System::String^ message, System::Drawing::Color color) {
    AppendTextSafe(message + Environment::NewLine, color);
}

void Cryptobot::LogPanel::AppendTextSafe(System::String^ text, System::Drawing::Color color) {
    if (logTextBox_->InvokeRequired) {
        logTextBox_->Invoke(gcnew Action<String^, Color>(this, &LogPanel::AppendTextSafe), text, color);
    }
    else {
        try {
            logTextBox_->SelectionStart = logTextBox_->TextLength;
            logTextBox_->SelectionLength = 0;
            logTextBox_->SelectionColor = color;
            logTextBox_->AppendText(text);
            logTextBox_->SelectionColor = System::Drawing::Color::Black;
            logTextBox_->ScrollToCaret();
        }
        catch (...) {

        }
    }
}