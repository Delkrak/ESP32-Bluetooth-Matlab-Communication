#include <string>

#include "helpers/board_task_name.h"
#include "helpers/Sheduler.h"

using namespace std;

string get_mac_address(void);

template <typename T>
string to_str(vector<T> v){
  std::stringstream ss;
  ss << "[";
  if(v.size() != 0)
    ss << "\"" << v[0] << "\"";
  for(int i = 1; i < v.size(); i++)
    ss << ", \"" << v[i] << "\"";
  ss << "]";
  return ss.str();
}