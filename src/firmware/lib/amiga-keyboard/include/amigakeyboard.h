#pragma once
#include "amigakbdparser.h"
namespace amiga_keyboard
{ 
    // Defined in the Amiga hardware manual
    #define AMIGA_KBD_WAIT_TIME_US (143)
    #define AMIGA_KBD_PULSE_WIDTH_MIN_US (1) 
    #define AMIGA_LOST_SYNC_CODE (0xF9)

    // How many attempts of sending recovery data before giving up
    #define AMIGA_SEND_KEY_RECOVERY_ATTEMPTS (20)

    // Send key State Machine states
    enum class SendKeySM
    {
        idle,
        data,
        clkdown,
        clkup,
        wait,
        end        
    };

    // Handshake State Machine states
    enum class HandshakeState 
    {
        waiting,
        pulse_active,
        pulse_done,
        pulse_error,
        timeout

    };

    enum class HandshakeStatus
    {
        ok,
        timeout,
        pulse_error,
        error

    };

    // Handshake error recovery states
    enum class HandshakeErrorState
    {
        send_1_bit,
        waiting,
        send_lost_sync,
        resend_key,
        force_end,
        done
    };

    class AmigaKeyboard
    {
        public:
            bool start_send_key(AmigaKey *key);
            void start_hard_reset();
            void stop_hard_reset();
        protected:
            bool send_key(AmigaKey* key, bool root);
            HandshakeStatus detect_handshake();
            bool handle_handshake_error(HandshakeStatus error, AmigaKey *key);
        private:
            SendKeySM state = SendKeySM::idle;
    };
}