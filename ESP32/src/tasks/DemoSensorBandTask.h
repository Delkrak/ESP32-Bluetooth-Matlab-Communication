#pragma once

#include "TaskInterface.h"

struct DemoSensorBandTask : TaskInterface {

  bool is_left_sensor_band;
  float count = 0;

  DemoSensorBandTask(bool is_left_sensor_band){
    this->is_left_sensor_band = is_left_sensor_band;
  }

  bool initialize(){
    if(is_left_sensor_band){
      description = "Left sensor band";
    } else {
      description = "Right sensor band";
    }
    low_frequency_sample_names = {
      "FSR1", "FSR2", "FSR3", "FSR4", "FSR5", "FSR6", "FSR7", "FSR8",
      "IMUx1", "IMUy1", "IMUz1", "IMUx2", "IMUy2", "IMUz2",
      "GYROx1", "GYROy1", "GYROz1", "GYROx2", "GYROy2", "GYROz2"
    };
    high_frequency_sample_names = {
      "EMG1", "EMG2", "EMG3", "EMG4"
    };
    return true;
  }
  bool process_message(const Message &incomming_message, function<bool(const Message&)> send_message_fun){
    if(incomming_message.is_command("reset_sensor_band_counters")){
      count = 0;
      send_message_fun(Message(incomming_message.command, {(float)true}));
      return true;
    }
    return false;
  }

  bool get_low_frequency_samples(vector<float> &low_frequency_samples, bool sample_frequency){
    low_frequency_samples.push_back(count + 0.01f); // FSR1
    low_frequency_samples.push_back(count + 0.02f); // FSR2
    low_frequency_samples.push_back(count + 0.03f); // FSR3
    low_frequency_samples.push_back(count + 0.04f); // FSR4
    low_frequency_samples.push_back(count + 0.05f); // FSR5
    low_frequency_samples.push_back(count + 0.06f); // FSR6
    low_frequency_samples.push_back(count + 0.07f); // FSR7
    low_frequency_samples.push_back(count + 0.08f); // FSR8

    low_frequency_samples.push_back(count + 0.09f); // IMUx1
    low_frequency_samples.push_back(count + 0.10f); // IMUy1
    low_frequency_samples.push_back(count + 0.11f); // IMUz1
    low_frequency_samples.push_back(count + 0.12f); // IMUx2
    low_frequency_samples.push_back(count + 0.13f); // IMUy2
    low_frequency_samples.push_back(count + 0.14f); // IMUz2

    low_frequency_samples.push_back(count + 0.15f); // GYROx1
    low_frequency_samples.push_back(count + 0.16f); // GYROy1
    low_frequency_samples.push_back(count + 0.17f); // GYROz1
    low_frequency_samples.push_back(count + 0.18f); // GYROx2
    low_frequency_samples.push_back(count + 0.19f); // GYROy2
    low_frequency_samples.push_back(count + 0.20f); // GYROz2

    return true;
  }
  bool get_high_frequency_samples(vector<float> &high_frequency_samples, bool sample_frequency){
    count++;

    high_frequency_samples.push_back(count + 0.21f); // EMG1
    high_frequency_samples.push_back(count + 0.22f); // EMG2
    high_frequency_samples.push_back(count + 0.23f); // EMG3
    high_frequency_samples.push_back(count + 0.24f); // EMG4

    return true;
  }
};