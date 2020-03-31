#include "helpers/board_task_name.h"

#include <iostream>
#include <sstream>
#include <string>

void ensure_initialized(){
  static bool initialized = false;
  if(!initialized) {
    EEPROM.begin(BOARD_TYPE_MAX_LENGTH + 1);
    initialized = true;
  }
}

string get_board_task_name(){
  ensure_initialized();

  char board_type_array[BOARD_TYPE_MAX_LENGTH + 1];
  size_t length = EEPROM.readString(
    BOARD_TYPE_EPROM_ADDRESS, board_type_array, BOARD_TYPE_MAX_LENGTH);
  if(BOARD_TYPE_MAX_LENGTH == length) return "";
  return string(board_type_array);
}

bool set_board_task_name(string new_board_type){
  ensure_initialized();

  if(BOARD_TYPE_MAX_LENGTH <= new_board_type.size()) return false;
  if(new_board_type != get_board_task_name() ){ 
    EEPROM.writeString(BOARD_TYPE_EPROM_ADDRESS, new_board_type.c_str());
    EEPROM.commit();
  }
   return true;
}