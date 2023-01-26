#include "amiga_platform.h"
#include "hardware/timer.h"

handshake_status detect_handshake()
{

  uint64_t start = time_us_64();
  uint64_t time = time_us_64();
  uint64_t pulse_start_time;
  bool pulse_found = false;
  handshake_state state = handshake_state::waiting;
  while (true)
  {
    switch(state) 
    {
      case handshake_state::waiting :
        
         if (amiga_get_data() == false) 
        {
          pulse_start_time = time_us_64();
          state = handshake_state::pulse_active;
        }
        else if (AMIGA_KBD_WAIT_TIME_US < time - start) 
        {
          state = handshake_state::timeout;
        }
      break;
      case handshake_state::pulse_active :
        if (amiga_get_data() == true)
        {
          if ( AMIGA_KBD_PULSE_WIDTH_MIN_US > time_us_64() - pulse_start_time)
          {
            state = handshake_state::pulse_error;
          }
          else
          {
            state = handshake_state::pulse_done;
          }

        }
      break;
      case handshake_state::pulse_done :
        return handshake_status::ok;
      break;
      case handshake_state::pulse_error :
        return handshake_status::pulse_error;
      break;
      case handshake_state::timeout :
        return handshake_status::timeout;
      break;

    }
    time = time_us_64();
  } 
  
  return handshake_status::error;
}
