#pragma once

#include <iostream>

#include "TaskInterface.h"

using namespace std;

struct DemoTask : TaskInterface {

  bool initialize(){
    description = "Demo task";
    low_frequency_sample_names = {};
    high_frequency_sample_names = {};
    return true;
  }

  bool process_message(const Message &incomming_message, function<bool(const Message&)> send_message_fun){
      // Return true if message is correct.

      // Different types of inputs:
      if(incomming_message.is_command("command_independent_of_arguments")){
        incomming_message.print();
        return true;

      } else if(incomming_message.is_command("command_with_2_numbers_and_any_strings", 2)) {
        float number_1 = incomming_message.numbers[0];
        float number_2 = incomming_message.numbers[1];
        cout << number_1 << ", " << number_2 << endl;
        return true;

      } else if(incomming_message.is_command("command_with_any_numbers_and_2_strings", -1, 2)) {
        string string_1 = incomming_message.strings[0];
        string string_2 = incomming_message.strings[1];
        return true;

      } else if(incomming_message.is_command("command_with_1_numbers_and_3_strings", 1, 3)) {
        float number_1 = incomming_message.numbers[0];
        string string_1 = incomming_message.strings[0];
        string string_2 = incomming_message.strings[1];
        string string_3 = incomming_message.strings[2];
        cout << number_1 << ", " << string_1 << ", " << string_2 << ", " << string_3 << endl;
        return true;

      } else if(incomming_message.is_command("some_command_with_answer")){
        send_message_fun(Message(incomming_message.command, {1.0f, 2.0f, 3.0f, 1234.0f}, {"a", "b", "c", "d"}));
        return true;
      }
      return false;
  }

  bool get_low_frequency_samples(vector<float> &low_frequency_samples, bool sending_signals){
    // What goes here:
    // * Closed loop motor controller
    // * Low frequency sampling of motor controller signal sampling
    // * Gyro and IMU sampling
    return true; // Return false if a critical error occured; otherwise return true.
  }
  bool get_high_frequency_samples(vector<float> &high_frequency_samples, bool sending_signals){
    // What goes here:
    // * EMG signal sampling
    return true;
  }
};