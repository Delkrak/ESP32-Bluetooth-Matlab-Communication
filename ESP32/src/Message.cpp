#include <stdio.h>
#include <Arduino.h>

#include "Message.h"
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <cstdint>
#include <cstring>

using namespace std;


Message::Message(const string &command, const vector<float> &numbers, const vector<string> &strings){
  this->strings = strings;
  this->command = command;
  this->numbers = numbers;
  this->timestamp = millis();
}

void Message::to_binary(vector<uint8_t> &data) const {
  data.clear();
  //data.reserve( 1 + command.size() + 4 * numbers.size());
  // Add commands and strings
  for(char c : command)
    data.push_back((uint8_t)c);
  for(auto &str : strings){
    data.push_back(STRING_SEPARATOR_CHARACTER);
    for(char c : str)
      data.push_back((uint8_t)c);
  }
  data.push_back(NUMBERS_START_CHARACTER);
  for(float number : numbers){
    uint8_t *byte_array = reinterpret_cast<uint8_t*>(&number); // reinterpret float to four uint8_t 
    for (size_t i = 0; i < 4; i++){
      data.push_back(byte_array[i]);                      // store reinterpreted data   
    }
  }
}

bool Message::from_binary(const vector<uint8_t> &data, Message &message){
  message.timestamp = millis();
  message.command.clear();
  message.strings.clear();

  size_t number_start_index;
  for(number_start_index = 0; number_start_index < data.size(); number_start_index++){
    if(data[number_start_index] == NUMBERS_START_CHARACTER)
      break;
  }
  if(number_start_index == data.size()) return false;

  // Extract command and strings
  string tmp_string = "";
  for(size_t index = 0; index <= number_start_index; index++){
    char c = data[index];

    if(c == STRING_SEPARATOR_CHARACTER || c == NUMBERS_START_CHARACTER){
      if(message.command.size() == 0){
        message.command = tmp_string;
      } else {
        message.strings.push_back(tmp_string);
      }
      tmp_string.clear();

    } else {
      tmp_string.push_back(c);
    }
  }

  // Extract numbers
  int number_bytes = data.size() - number_start_index - 1;
  if(number_bytes % 4 != 0) return false;
  
  message.numbers.resize(number_bytes / 4);
  memcpy(&message.numbers[0], (void*)&data[number_start_index + 1], number_bytes);
  return true;
}

void Message::print(string name) const {
  if(0 < name.size()) cout << name << ": ";
  cout << command << " -> [";
  bool first = true;
  for(int i = 0; i < numbers.size(); i++){
    if( first ) cout << ", ";
    cout << numbers[i];
    first = false;
  }
  for(int i = 0; i < strings.size(); i++){
    if( first ) cout << ", ";
    cout << "\"" << strings[i] << "\"";
    first = false;
  }
  cout << "]" << endl;
}

bool Message::is_command(const string &some_command, int size_of_numbers, int size_of_strings) const {
  if( 0 <= size_of_strings && strings.size() != size_of_strings) return false;
  if( 0 <= size_of_numbers && numbers.size() != size_of_numbers) return false;
  return command == some_command;
}

bool Message::operator==(const Message &other){
  bool command_is_equal = command == other.command;
  bool numbers_are_equal = numbers == other.numbers;
  bool strings_are_equal = strings == other.strings;
  return command_is_equal && numbers_are_equal && strings_are_equal;
}


void test_message(string name, const Message &message){
  vector<uint8_t> message_binary;
  message.to_binary(message_binary);

  Message message_recovered;
  
  if(!Message::from_binary(message_binary, message_recovered)){
    cout << "Failed to correctly convert message!" << endl;
  }

  cout << "Test '" << name << "' pass state: " << (message_recovered == message) << endl;

}

bool test_message(void){
  string command = "myCommand";
  vector<float> numbers = {1.0f, 2.0f, 3.0f, 1000.0f};
  vector<string> strings = {"a", "b", "c", "something long"};

  Message m1(command, numbers, strings), m2(command, numbers, strings), m3("my", {1.0f}, {"HMM!"});
  cout << "Area equal test #1: " << (m1 == m2) << endl;
  cout << "Area equal test #2: " << (m1 == m2) << endl;
  cout << "Area equal test #3: " << !(m1 == m3) << endl;

  test_message("#1", Message(command));
  test_message("#2", Message(command, numbers));
  test_message("#3", Message(command, {}, strings));
  test_message("#4", Message(command, numbers, strings));

  return true;
}