#pragma once
#include "quokkamiga_gpio.h"

#define AMIGA_KBD_WAIT_TIME_US (143)
#define AMIGA_KBD_PULSE_WIDTH_MIN_US (1) 

enum class handshake_state 
{
    waiting,
    pulse_active,
    pulse_done,
    pulse_error,
    timeout

};

enum class handshake_status
{
    ok,
    timeout,
    pulse_error,
    error

};

handshake_status detect_handshake();

inline void amiga_set_data(bool high)
{
    if (high)
    {
        AMIGA_DATA_OUT_HIGH();
    }
    else 
    {
        AMIGA_DATA_OUT_LOW();
    }
}

inline bool amiga_get_data()
{
    return AMIGA_DATA_IN_GET();
}

inline void amiga_set_clock(bool high)
{
    if (high)
    {
        AMIGA_CLK_OUT_HIGH();
    }
    else 
    {
        AMIGA_CLK_OUT_LOW();
    }
}