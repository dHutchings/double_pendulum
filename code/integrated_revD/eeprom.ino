#include <EEPROM.h>

int addr = 0; //just store the speed up front at the beginning of the EEPROM

void load_from_eeprom()
{
  #if RESET_EEPROM
  reset_eeprom();
  #endif
  
  long push_time_in_eeprom;
  EEPROM.get(0,push_time_in_eeprom);
  if(push_time_in_eeprom < MIN_PUSH || push_time_in_eeprom > MAX_PUSH)
  {
    reset_eeprom();
    #if DEBUG_PRINTS
    Serial.println("EEPROM Value is out of bands, resetting with the nominal value.");
    precise_idle(1000);
    
    #endif

    push_time_us = NOMINAL_PUSH;

  }
  else
  {
    #if DEBUG_PRINTS
    Serial.print("Push time in EEPROM:");
    Serial.println(push_time_in_eeprom);
    precise_idle(1000);
    #endif
    
    push_time_us = push_time_in_eeprom;
  }
}

void reset_eeprom()
{
  EEPROM.put(addr, NOMINAL_PUSH);
}

void setup_eeprom()
{
  EEPROM.get(0,push_time_us);
}

void eeprom_current_speed()
{
  EEPROM.put(addr, push_time_us);
}
