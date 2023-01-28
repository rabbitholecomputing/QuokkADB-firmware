#include "amigakeyboard.h"
#include "amiga_platform.h"
namespace amiga_keyboard
{
    bool AmigaKeyboard::start_send_key(AmigaKey *key)
    {
        return send_key(key, true);
    }

    bool AmigaKeyboard::send_key(AmigaKey *key, bool root)
    {
        uint8_t bit_count = 8;
        state = SendKeySM::idle;
        HandshakeStatus handshake;
        amiga_set_data(true);
        amiga_set_clock(true);
        bool success = false;

        while (state != SendKeySM::end)
        {
            switch (state)
            {
            case SendKeySM::idle:
                // wait for host to free the data line
                while (false == amiga_get_data());
                state = SendKeySM::data;
                break;

            case SendKeySM::data:
                // data is active low
                bit_count--;
                amiga_set_data(!(0x01 & (key->rotatedKeyCode >> bit_count)));
                sleep_us(20);
                state = SendKeySM::clkdown;
            break;

            case SendKeySM::clkdown:
                amiga_set_clock(false);
                sleep_us(20);
                state = SendKeySM::clkup;
            break;

            case SendKeySM::clkup:
                amiga_set_clock(true);
                if (bit_count == 0)
                {
                    sleep_us(20);
                    amiga_set_data(true);
                    handshake = detect_handshake();
                    if (handshake == HandshakeStatus::ok)
                    {
                        success = true;
                    }
                    else if(root)
                    {
                        handle_handshake_error(handshake, key);
                    }
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
        return success;
    }


    HandshakeStatus AmigaKeyboard::detect_handshake()
    {

        uint64_t start = time_us_64();
        uint64_t time = time_us_64();
        uint64_t pulse_start_time;
        bool pulse_found = false;
        HandshakeState state = HandshakeState::waiting;
        while (true)
        {
            switch (state)
            {
            case HandshakeState::waiting:

                if (amiga_get_data() == false)
                {
                    pulse_start_time = time_us_64();
                    state = HandshakeState::pulse_active;
                }
                else if (AMIGA_KBD_WAIT_TIME_US < time - start)
                {
                    state = HandshakeState::timeout;
                }
            break;
            case HandshakeState::pulse_active:
                if (amiga_get_data() == true)
                {
                    if (AMIGA_KBD_PULSE_WIDTH_MIN_US > time_us_64() - pulse_start_time)
                    {
                        // host handshake pulse was shorter than 1us
                        state = HandshakeState::pulse_error;
                    }
                    else
                    {
                        state = HandshakeState::pulse_done;
                    }
                }
            break;
            case HandshakeState::pulse_done:
                return HandshakeStatus::ok;
            break;
            case HandshakeState::pulse_error:
                return HandshakeStatus::pulse_error;
            break;
            case HandshakeState::timeout:
                return HandshakeStatus::timeout;
            break;
            }
            time = time_us_64();
        }

        return HandshakeStatus::error;
    }

    bool AmigaKeyboard::handle_handshake_error(HandshakeStatus error, AmigaKey *key)
    {
        AmigaKey lost_sync_key_code;
        HandshakeErrorState state = HandshakeErrorState::send_1_bit;
        HandshakeStatus status;
        static uint8_t limit_recovery_attempts = 0;
        static uint8_t unwind_recursion = 0;

        if (error == HandshakeStatus::timeout || error == HandshakeStatus::pulse_error)
        {
            while (true)
            {
                switch (state)
                {
                case HandshakeErrorState::send_1_bit :
                    // clear clock and data
                    amiga_set_clock(true);
                    amiga_set_data(true);

                    amiga_set_data(false);
                    sleep_us(20);
                    amiga_set_clock(false);
                    sleep_us(20);
                    amiga_set_clock(true);
                    sleep_us(20);
                    amiga_set_data(true);
                    state = HandshakeErrorState::waiting;
                break;
                case HandshakeErrorState::waiting:
                    status = detect_handshake();
                    if (status == HandshakeStatus::ok)
                    {
                        state = HandshakeErrorState::send_lost_sync;
                    }
                    else
                    {
                        state = HandshakeErrorState::send_1_bit;
                    }
                break;
                case HandshakeErrorState::send_lost_sync:
                    if (limit_recovery_attempts >= AMIGA_SEND_KEY_RECOVERY_ATTEMPTS)
                    {
                        state = HandshakeErrorState::force_end;
                    }
                    else 
                    {
                        lost_sync_key_code.isKeyDown = false;
                        lost_sync_key_code.rotatedKeyCode = AMIGA_LOST_SYNC_CODE;
                        limit_recovery_attempts++;
                        if (send_key(&lost_sync_key_code, false))
                        {
                            state = HandshakeErrorState::resend_key;
                        }
                        else 
                        {
                            state = HandshakeErrorState::send_1_bit;
                        }
                    } 
                break;
                case HandshakeErrorState::resend_key:
                    if (limit_recovery_attempts >= AMIGA_SEND_KEY_RECOVERY_ATTEMPTS)
                    {
                        state = HandshakeErrorState::force_end;
                    }
                    else 
                    {
                        limit_recovery_attempts++;
                        if (send_key(key, false))
                        {
                            state = HandshakeErrorState::done;
                        }
                        else
                        {
                            state = HandshakeErrorState::send_1_bit;
                        }
                    } 
                break;
                case HandshakeErrorState::force_end :
                        return false;
                break;
                case HandshakeErrorState::done :
                    return true;
                break;
                }

            }
        }
        // should never get here
        return false;
    }
    void AmigaKeyboard::start_hard_reset() 
    {
        amiga_set_clk_bit_1();
    }

    void AmigaKeyboard::stop_hard_reset()
    {
        amiga_set_clk_bit_0();
    }
}


