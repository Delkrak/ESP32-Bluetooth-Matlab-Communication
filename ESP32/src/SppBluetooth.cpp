#include <iostream>
#include <algorithm>
#include <functional>

#include "SppBluetooth.h"
#include "../lib/cobs-c/cobs.h"


void print_array(string name, const vector<uint8_t> &arr){
  cout << name << ": [";
  for(int i = 0; i < arr.size(); i++){
    if( 0 < i ) cout << ", ";
    cout << (int)arr[i];
  }
  cout << "]" << endl;
}

// Cobs is an 8-bit encoding sheme that is very efficient with a minimum of overhead.
// Read more about it here: https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
bool cobs_encode(const vector<uint8_t> &input, vector<uint8_t> &output){
  output.resize(1 + input.size() + (1 + input.size() / 254)); // Worst case required buffer
  auto encode_result = cobs_encode(&output[0], output.size(), &input[0], input.size());
  output.resize(encode_result.out_len);
  output.push_back('\0');

  bool status_ok = encode_result.status == COBS_ENCODE_OK;
  if(!status_ok){
    cout << "Status code: " << (int) encode_result.status << endl;
    print_array("Failed to encode", input);
    print_array("Failed to encode (output)", output);
  }
  return status_ok;
}

bool cobs_decode(const vector<uint8_t> &input, vector<uint8_t> &output){
  output.resize(input.size());
  for(int i = 0; i < output.size(); i++)
    output[i] = 244;
  auto decode_result = cobs_decode(&output[0], output.size(), &input[0], input.size());
  output.resize(decode_result.out_len);

  bool status_ok = decode_result.status == COBS_DECODE_OK;
  if(!status_ok){
    cout << "Status code: " << (int) decode_result.status << endl;
    print_array("Failed to decode", input);
    print_array("Failed to decode (output)", output);
  }
  return status_ok;
}

SppBluetooth::SppBluetooth(string server_name){
  binary_buffer.reserve(1024);
  encoded_binary_buffer.reserve(1024);
  binary_read_buffer.reserve(1024);
  decoded_binary_read_buffer.reserve(1024);

  this->server_name = server_name;

  bluetooth_server.begin(String(this->server_name.c_str()));
}

bool SppBluetooth::send(const Message &message){
  message.to_binary(binary_buffer);
  if(cobs_encode(binary_buffer, encoded_binary_buffer)){
    return encoded_binary_buffer.size() == bluetooth_server.write(
      &encoded_binary_buffer[0], encoded_binary_buffer.size());
  }
  return false;
}

bool SppBluetooth::get_next_message(Message &message){
  while( 0 < bluetooth_server.available()){
    uint8_t byte = bluetooth_server.read();

    if(byte == '\0'){
      bool decoding_results = cobs_decode(binary_read_buffer, decoded_binary_read_buffer);
      binary_read_buffer.clear();
      if(!decoding_results) return false;

      bool conversion_result = Message::from_binary(decoded_binary_read_buffer, message);
      if(!conversion_result){
        print_array("Error converting to message", decoded_binary_read_buffer);
      }
      return conversion_result;
    }else{
      binary_read_buffer.push_back(byte);
    }
  }
  return false;
}

bool SppBluetooth::is_connected() {
  return bluetooth_server.hasClient();
}


