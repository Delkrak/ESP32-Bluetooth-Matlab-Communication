#pragma once

#include "Message.h"
#include <functional>

struct TaskInterface {
    string description;
    vector<string> low_frequency_sample_names, high_frequency_sample_names;

    virtual bool initialize() = 0;
    virtual bool process_message(const Message &incomming_message, function<bool(const Message&)> send_message_fun) = 0;

    virtual bool get_low_frequency_samples(vector<float> &low_frequency_samples, bool sending_signals) = 0;
    virtual bool get_high_frequency_samples(vector<float> &high_frequency_samples, bool sending_signals) = 0;
};