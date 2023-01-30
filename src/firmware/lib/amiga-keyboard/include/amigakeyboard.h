#pragma once
#include "amigakbdparser.h"
namespace amiga_keyboard
{ 
    // Defined in the Amiga hardware manual
    #define AMIGA_KBD_WAIT_TIME_US (143)
    #define AMIGA_KBD_PULSE_WIDTH_MAX_US (AMIGA_KBD_WAIT_TIME_US + 85)
    #define AMIGA_KBD_PULSE_WIDTH_MIN_US (1) 
    #define AMIGA_LOST_SYNC_CODE (0xF9)
    #define AMIGA_SOFT_RESET_WAIT_BEGIN_US (250 * 1000)
    #define AMIGA_SOFT_RESET_WAIT_FINISH_US (10 * 1000 * 1000)
    #define AMIGA_HARD_RESET_MIN (500 * 1000)
    // arbitrary wait timeout
    #define AMIGA_HARD_RESET_TIMEOUT (30 * 1000 * 1000)

    // How many attempts of sending recovery data before giving up
    #define AMIGA_SEND_KEY_RECOVERY_ATTEMPTS (20)


    enum class SendKeyType
    {
        root,
        soft_reset_root,
        soft_reset_skip_handshake,
        skip_handshake_error_handling
    };
    // Send key State Machine states
    enum class SendKeySM
    {
        idle,
        data,
        clkdown,
        clkup,
        soft_reset,
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
            AmigaKeyboard(AmigaKbdRptParser* kbd_rpt_parser);
            bool start_send_key(AmigaKey *key);
            uint64_t soft_reset();
            bool hard_reset(uint64_t soft_reset_duration);
        protected:
            bool send_key(const AmigaKey* key, const SendKeyType send_type);
            HandshakeStatus detect_handshake();
            bool handle_handshake_error(const HandshakeStatus error, const AmigaKey *key, bool is_soft_reset);
            bool detect_soft_reset();
            AmigaKbdRptParser *parser;
        private:
            SendKeySM state = SendKeySM::idle;
    };
}