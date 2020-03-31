#include <Arduino.h>
#include <sstream>

#include "helpers/helpers.h"

string get_mac_address(void){
	uint64_t chipid = ESP.getEfuseMac();
  char chipid_buffer[50];
  sprintf(chipid_buffer, "%04X%08X", (uint16_t)(chipid>>32), (uint32_t)chipid);
  return string(chipid_buffer);
}