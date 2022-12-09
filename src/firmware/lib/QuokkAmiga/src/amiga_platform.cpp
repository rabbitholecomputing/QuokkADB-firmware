#include "amiga_platform.h"
bool detect_handshake()
{
    if (amiga_get_data())
    {
        return true;
    }
    // @TODO implement 1us handshake
    return false;
}