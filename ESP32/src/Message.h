#pragma once

#include <vector>
#include <string>

using namespace std;

#define STRING_SEPARATOR_CHARACTER 29 // 29 is the default group separator charactor
#define NUMBERS_START_CHARACTER 30
/*

*/
struct Message{
  string command;
  vector<float> numbers;
  vector<string> strings;
  unsigned long timestamp;

  Message(const string &command = "", const vector<float> &numbers = {}, const vector<string> &strings = {});

  bool is_command(const string &some_command, int size_of_numbers = -1, int size_of_strings = -1) const;
  void to_binary(vector<uint8_t> &data) const;
  static bool from_binary(const vector<uint8_t> &data, Message &message);

  void print(string name = "") const;
  bool operator==(const Message &other);
};


bool test_message(void);