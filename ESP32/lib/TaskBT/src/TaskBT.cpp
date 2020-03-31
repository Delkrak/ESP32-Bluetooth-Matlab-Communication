#include "Arduino.h"
#include "TaskBT.h"
#include <iostream>
#include <vector>

uint8_t TaskBuffer[10];

std::string str;
std::string task;

char ReinterpretCast(int Value){
    int* p = new int(Value); 
    char* ch = reinterpret_cast<char*>(p); 
    return *ch;
}

void GetTask(BluetoothSerial &ConnectionBT){
    char c = ReinterpretCast(ConnectionBT.read());
    str.push_back(c);
    std::size_t found = str.find("//");
    if(found != std::string::npos){
        task.erase(task.begin(),task.end());
        task = str.substr(0,found);
        cout << "Task found and is: " << task << endl;
        str.erase(str.begin(),str.end());
    }
}



