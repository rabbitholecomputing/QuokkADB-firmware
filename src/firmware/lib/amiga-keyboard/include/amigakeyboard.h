#pragma once
#include "amigakbdparser.h"
namespace amiga_keyboard
{ 
    // Statem Machine status
    enum class SendKeySM
    {
        idle,
        data,
        clkdown,
        clkup,
        wait,
        end        
    };

    class AmigaKeyboard
    {
        public:
            void send_key(AmigaKey* key);
        private:
            SendKeySM state = SendKeySM::idle;

    };
}