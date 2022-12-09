#include "amigakeyboard.h"
#include "amiga_platform.h"
namespace amiga_keyboard
{ 


    void AmigaKeyboard::send_key(AmigaKey* key)
    {
        uint8_t bit_count = 0;
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
                    bit_count++;
                    // data is active low
                    if(bit_count >= 8)
                    {
                        // send the LSB for the last transmission (button up/down status)
                        amiga_set_data(!(0x01 & (key->rotatedKeyCode)));
                    }
                    else
                    {
                        // unwrap the 7-bit keycode
                        amiga_set_data(!(0x01 & (key->rotatedKeyCode >> bit_count)));
                    }
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
                    if (bit_count >= 8)
                    {
                        amiga_set_data(true);
                        while (!detect_handshake());
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