
#pragma once

#include <vector>
#include <string>

#include <Arduino.h>
#include "BluetoothSerial.h"
#include "Message.h"


using namespace std;

struct SppBluetooth{
    string server_name;

    BluetoothSerial bluetooth_server;
    vector<uint8_t> binary_buffer, encoded_binary_buffer;
    vector<uint8_t> binary_read_buffer, decoded_binary_read_buffer;

    SppBluetooth(string server_name);

    bool send(const Message &message);
    bool get_next_message(Message &message);

    bool is_connected();
};

