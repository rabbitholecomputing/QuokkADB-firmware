#pragma once
#include "quokkamiga_gpio.h"


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