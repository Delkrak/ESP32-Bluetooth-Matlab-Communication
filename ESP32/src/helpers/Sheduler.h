#pragma once

#include <Arduino.h>
#include <iostream>

using namespace std;


struct Sheduler{
  unsigned long time_next_us = 0;

  unsigned long total_periods_behind = 0;
  float total_time_behind = 0.0;
  bool verbose;
  unsigned long last_print_us = 0;

  Sheduler(bool verbose = true){
    verbose = verbose;
    reset();
  }

  bool shedule(float target_frequency){
    if( target_frequency <= 0.0f) return true;

    unsigned long time_now_us = micros();

    unsigned long target_period_us = (1.0f / target_frequency) * 1e6f;
    time_next_us += target_period_us;
    if (time_next_us < time_now_us) {
        int periods_behind = 1 + (int)((time_now_us - time_next_us) / target_period_us);
        time_next_us += periods_behind * target_period_us;

        total_periods_behind += periods_behind;
        total_time_behind += (float)(time_now_us - time_next_us) * 1e-6f;
        if(verbose && last_print_us + target_period_us * (unsigned long)10 < time_now_us){ // limit print to 10 times a second
           cout << "Sheduler can't keep up! Skipping " << periods_behind << " additional samples. ";
           cout << "Total skipped: [" << total_periods_behind << " samples; " << total_time_behind << "]" << endl;
           last_print_us = time_now_us;
        }
        return false;
    }

    int time_to_wait_us = time_next_us - time_now_us;
    if(0 < time_to_wait_us) delayMicroseconds(time_to_wait_us);
    return true;
  }

  void reset(void){
    time_next_us = micros();
    total_periods_behind = 0;
    total_time_behind = 0.0f;
  }
};