#include <iostream>
#include <sstream>
#include <string>
#include <Arduino.h>

#include <functional>

#include "helpers/helpers.h"
#include "SppBluetooth.h"
#include "tasks/task_list.h"


SppBluetooth *bluetooth_server = NULL;
Message incomming_message;
Sheduler sheduler(true);
TaskInterface *active_task = NULL;

float sample_frequency = 1000.0f;
int send_signals_ratio = 10;
unsigned long frame = 0;
bool send_signals = false;
bool is_connected_last = false;
vector<float> signals_buffer;
vector<string> lf_signal_names, hf_signal_names;
vector<string> potential_tasks_names;

bool debug_print = false;

void setup() {
  // Open serial port
  Serial.begin(115200);
  Serial.println("");
  cout << "Initializing..." << endl;

  // Board task name
  string board_type = get_board_task_name();
  cout << " * Board task name: \"" << board_type << "\"" << endl;

  // Setup Bluetooth
  string bluetooth_name = get_mac_address() + "@Exo-Aider";
  cout << " * Bluetooth name: \"" << bluetooth_name << "\"" << endl;
  bluetooth_server = new SppBluetooth(bluetooth_name);

  // Determine what task to do
  std::map<string, TaskInterface*> potential_tasks;
  get_potential_tasks(potential_tasks);
  active_task = NULL;
  for(auto potential_task : potential_tasks){
    potential_tasks_names.push_back(potential_task.first);
    if(active_task == NULL && potential_task.first == board_type){
      active_task = potential_task.second;
    }
  }
  cout << " * " << potential_tasks_names.size() << " potential tasks: " << to_str(potential_tasks_names) << endl;

  if(active_task != NULL){
    if(active_task->initialize()){
      // Low frequency signal names
      lf_signal_names.push_back("n"); 
      lf_signal_names.push_back("t");
      for(auto name : active_task->low_frequency_sample_names) 
        lf_signal_names.push_back(name);
      hf_signal_names = active_task->high_frequency_sample_names;

      cout << " * " << lf_signal_names.size() << " low frequency signals: " << to_str(lf_signal_names) << endl;
      cout << " * " << hf_signal_names.size() << " high frequency signals: " << to_str(hf_signal_names) << endl;

    }else{
      cout << " * Error during initialization of task!" << endl;
      active_task = NULL;
    }
  }else{
    cout << " * Warning: Board task does not match any potential task." << endl;
  }
  if(active_task == NULL){
    cout << " * Warning: No task is currently running." << endl;
  }

  // Reset the sheduler
  sheduler.reset();
  cout << "Initialized!" << endl;
}

void loop() {  
  frame++;
  sheduler.shedule(sample_frequency);

  if(active_task != NULL && bluetooth_server != NULL){
    bool is_low_frequency_frame = frame % (unsigned long)send_signals_ratio == 0;
    size_t expected_post_signals_buffer_size;

    // High frequency sampling
    expected_post_signals_buffer_size = signals_buffer.size() + active_task->high_frequency_sample_names.size();
    if(!active_task->get_high_frequency_samples(signals_buffer, sample_frequency)){
      cout << "Error executing low frequency sample task!" << endl;
      return;
    }
    if(signals_buffer.size() != expected_post_signals_buffer_size){
      cout << "Incorrect high frequency signals buffer size! " << 
          signals_buffer.size() << " != " << expected_post_signals_buffer_size  << endl;
      return;
    }

    // Low frequency sampling
    if(is_low_frequency_frame){
      int expected_pre_signals_buffer_size = active_task->high_frequency_sample_names.size() * send_signals_ratio;
      if( signals_buffer.size() != expected_pre_signals_buffer_size)
        cout << " Incorrect signals_buffer size pre low frequency. " << signals_buffer.size() << " != " << expected_pre_signals_buffer_size << endl;

      // Add hedder
      signals_buffer.push_back((float)frame);
      signals_buffer.push_back((float)((double)millis() / 1000.0));

      expected_post_signals_buffer_size = signals_buffer.size() + active_task->low_frequency_sample_names.size();
      if(!active_task->get_low_frequency_samples(signals_buffer, sample_frequency)){
        cout << "Error executing low frequency sample task!" << endl;
        return;
      }
      if(signals_buffer.size() != expected_post_signals_buffer_size){
        cout << "Incorrect low frequency signals buffer size! " << 
          signals_buffer.size() << " != " << expected_post_signals_buffer_size  << endl;
        return;
      }
    }

    // Send signals
    if(is_low_frequency_frame){
      if(debug_print && send_signals) cout << frame << "- E6 - send signal buffer" << endl;

      // * Send the signals buffer
      if(bluetooth_server->is_connected() && send_signals){
        int expected_size = lf_signal_names.size() + send_signals_ratio * hf_signal_names.size();
        if(expected_size != signals_buffer.size()) 
          cout << "'signals_buffer.size()' is incorrect size: " << signals_buffer.size() << " != " << expected_size << endl;

        if(debug_print && send_signals) cout << frame << "- E7 - sending signal buffer" << endl;
        bluetooth_server->send(Message("sig", signals_buffer));
      }

      // * Clear signal buffer and add header information
      signals_buffer.clear();
    }
  }else{
    if(debug_print) cout << "something is wrong with the task.." << endl;
  }

  function<bool(const Message&)> send_message_fun = [&](const Message &message){
    return bluetooth_server->send(message);
  };
  while(bluetooth_server->get_next_message(incomming_message)){
    if(incomming_message.is_command("ping")){
      send_message_fun(Message(incomming_message.command, {(float)true, (float)incomming_message.timestamp, (float)((double)millis() / 1000.0)}));

    } else if(active_task != NULL && active_task->process_message(incomming_message, send_message_fun)){
      // The message was processed by the active task

    } else if(incomming_message.is_command("set_board_task_name", 0, 1)){
      string new_board_type_string = incomming_message.strings[0];
      send_message_fun(Message(incomming_message.command, {(float)set_board_task_name(new_board_type_string)}));
      cout << "New board type: " << get_board_task_name() << endl;

    } else if(incomming_message.is_command("get_board_task_name")){ 
      string type = get_board_task_name();
      send_message_fun(Message(incomming_message.command, {(float)true}, {type}));

    } else if(incomming_message.is_command("restart", 0)){
      cout << "Restarting device.. " << endl;
      send_message_fun(Message(incomming_message.command, {true}));
      delay(100);
      ESP.restart();

    } else if(incomming_message.is_command("get_sheduler_periods_behind", 0)){
      send_message_fun(Message(incomming_message.command, {(float)true, (float)sheduler.total_periods_behind}));

    } else if(incomming_message.is_command("set_sample_frequency", 1) && 1.0f <= incomming_message.numbers[0]){
      sample_frequency = incomming_message.numbers[0];
      send_message_fun(Message(incomming_message.command, {(float)true, sample_frequency}));
      cout << "Setting 'sample_frequency' = " << sample_frequency << endl;

    } else if(incomming_message.is_command("get_sample_frequency")){
      send_message_fun(Message(incomming_message.command, {sample_frequency}));

    } else if(incomming_message.is_command("set_send_signals_ratio", 1) && 1.0f <= incomming_message.numbers[0]){
      send_signals_ratio = incomming_message.numbers[0];
      send_message_fun(Message(incomming_message.command, {(float)true, (float)send_signals_ratio}));
      cout << "Setting 'send_signals_ratio' = " << send_signals_ratio << endl;

    } else if(incomming_message.is_command("get_send_signals_ratio")){
      send_message_fun(Message(incomming_message.command, {(float)send_signals_ratio}));

    } else if(incomming_message.is_command("set_send_signals", 1)) {
      send_signals = incomming_message.numbers[0] == (float)true;
      send_message_fun(Message(incomming_message.command, {(float)true}));
      cout << "Setting 'send_signals' = " << send_signals << endl;

    } else if(incomming_message.is_command("get_send_signals")) {
      send_message_fun(Message(incomming_message.command, {(float)send_signals}));

    } else if(incomming_message.is_command("get_lf_signal_names", 0)) {
      send_message_fun(Message(incomming_message.command, {}, lf_signal_names));

    } else if(incomming_message.is_command("get_hf_signal_names", 0)) {
      send_message_fun(Message(incomming_message.command, {}, hf_signal_names));

    } else if(incomming_message.is_command("get_task_description", 0)) {
      string task_description = "";
      if(active_task != NULL) task_description = active_task->description;
      send_message_fun(Message(incomming_message.command, {}, {task_description}));

    } else if(incomming_message.is_command("relay")){
      send_message_fun(incomming_message);

    } else {
      send_message_fun(Message(incomming_message.command, {(float)false}));
      incomming_message.print("Unrecognized command");
    }
  }

  bool is_connected_now = bluetooth_server->is_connected();
  if(is_connected_now != is_connected_last){
    if(is_connected_now){
      cout << "Connected!" << endl;
    }else{
      cout << "Lost connection.." << endl;
      send_signals = false; // Stop sending after losing connection
    }
  }
  is_connected_last = is_connected_now;
}

