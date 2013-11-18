#include "packet.h"

#include <iostream>
#include <fstream>
//#include <iomanip>

using namespace std;

int main () {
  char* data = {"Hello World"};
  Packet p = {0};
  p.seq_num = 128;
  p.ack_num = 128;
  p.data_len = 12;
  p.ACK = false;
  p.FIN = true;
  
  for(int i = 0; i < 12; i++) {
    p.data[i] = data[i];
  }

  //char* char_array = (char*)&p;
  char* char_array = reinterpret_cast<char*>(&p);
  for(int i = 0; i < 128; ++i) {
    cout << std::hex << (int)char_array[i] << endl;
  }

  ifstream send_file;
  send_file.open("test.txt", ifstream::in);

  if(send_file) {
    cout << "true" << endl;
  } else {
    cout << "false" << endl;
  }

  send_file.read(char_array, 100);

  if(send_file) {
    cout << "true" << endl;
  } else {
    cout << "false" << endl;
  }

  return 0;
}
