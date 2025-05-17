#pragma once

#include <Windows.h>

namespace Cryptobot {

    public ref class LogPanel : public System::Windows::Forms::Panel
    {
    public:
        LogPanel();
        void Log(System::String^ message);
        void Log(System::String^ message, System::Drawing::Color color);

    private:
        System::Windows::Forms::RichTextBox^ logTextBox_;
        void AppendTextSafe(System::String^ text, System::Drawing::Color color);
    };
}