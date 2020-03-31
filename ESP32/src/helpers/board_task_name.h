#pragma once

#include <EEPROM.h>
#include <string>

#define BOARD_TYPE_EPROM_ADDRESS 0
#define BOARD_TYPE_MAX_LENGTH 64

using namespace std;

string get_board_task_name();

bool set_board_task_name(string new_board_task_name);