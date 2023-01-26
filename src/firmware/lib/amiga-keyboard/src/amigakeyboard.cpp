#include "amigakeyboard.h"
#include "amiga_platform.h"
namespace amiga_keyboard
{ 


    void AmigaKeyboard::send_key(AmigaKey* key)
    {
        uint8_t bit_count =  8;
        state = SendKeySM::idle;
        amiga_set_data(true);
        amiga_set_clock(true);
        while(state != SendKeySM::end)
        {
            switch(state)
            {
                case SendKeySM::idle :
                    // wait for host to free the data line
                    while (false == amiga_get_data() );
                    state = SendKeySM::data;
                break;

                case SendKeySM::data :  
                    // data is active low
                    bit_count--;
                    amiga_set_data(!(0x01 & (key->rotatedKeyCode >> bit_count)));
                    sleep_us(20);
                    state = SendKeySM::clkdown;
                break;

                case SendKeySM::clkdown :
                    amiga_set_clock(false);
                    sleep_us(20);
                    state = SendKeySM::clkup;
                break;

                case SendKeySM::clkup :
                    amiga_set_clock(true);
                    if (bit_count == 0)
                    {
                        sleep_us(20);
                        amiga_set_data(true);
                        detect_handshake();
                        state = SendKeySM::end;
                    }
                    else 
                    {
                        sleep_us(20);
                        state = SendKeySM::data;      
                    }
                break;

                default:
                    state = SendKeySM::end;
            }
        }

    }
}