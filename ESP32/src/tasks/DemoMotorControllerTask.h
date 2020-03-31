#pragma once

#include "TaskInterface.h"

struct DemoMotorControllerTask : TaskInterface {

  bool initialize(){
    low_frequency_sample_names = {
      "u", "y", "r", 
      "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20"
    };
    high_frequency_sample_names = {};
    description = "Motor Controller";
    return true;
  }

  bool process_message(const Message &incomming_message, function<bool(const Message&)> send_message_fun){
    if(incomming_message.is_command("u", 1)) {
      u_next_n = incomming_message.numbers[0]; // No reply!
      return true;

    } else if(incomming_message.is_command("u", 2)) {
      u_next_n = incomming_message.numbers[0]; // No reply!
      r_next_n = incomming_message.numbers[1]; // No reply!
      return true;

    } else if(incomming_message.is_command("get_u")) {
      send_message_fun(Message(incomming_message.command, {(float)u_next_n}));
      return true;

    }
    return false;
  }

  bool get_low_frequency_samples(vector<float> &low_frequency_samples, bool sending_signals){
    low_frequency_count++;
    u_n = u_next_n;
    r_n = r_next_n;

    low_frequency_samples.push_back(u_n);
    low_frequency_samples.push_back(y_n);
    low_frequency_samples.push_back(r_n);
    for(int i = 2; i < 20; i++) low_frequency_samples.push_back((float)i / 100.0f);

    // CLOSED LOOP CONTROL OF THE MOTOR CONTROLLER GOES HERE

    return true;
  }

  bool get_high_frequency_samples(vector<float> &high_frequency_samples, bool sending_signals){
    high_frequency_count++;
    simulate_system(); // Only simulate

    return true;
  }

  unsigned long low_frequency_count, high_frequency_count;

  unsigned long n = 0;
  double y_n = 0, y_n1 = 0, u_next_n = 0, u_n = 0, r_next_n = 0, r_n = 0, t_last = -1;
  void simulate_system(){
    double a = 10.0, b = a;

    double t_now = (double)micros() * 1e-6;
    if(t_last < 0) t_last = t_now;
    double Ts = t_now - t_last;
    t_last = t_now;

    y_n = y_n1;
    double A = (1.0 - Ts * a);
    double B = Ts * b;
    y_n1 = y_n * A + u_n * B; 
    

    n += 1;
    if( n % 1000 == 0){
      cout << "high_frequency_count = " << high_frequency_count;
      cout << "; low_frequency_count = " << low_frequency_count;
      cout << "; t_now = " << t_now;
      cout << "; Ts = " << Ts;
      cout << "; A = " << A;
      cout << "; B = " << B;
      cout << "; y = " << y_n;
      cout << "; u = " << u_n;
      cout << endl;
    }
  }
};