#include "udp_socket.h"
#include "receiver_state_machine.h"

#include <iostream>

#define LOCAL_PORT 5051

using namespace std;

int main(int argc, char* argv[]) {
  if(argc != 3) {
    cout << "Need 2 args: <receiver-port> <loss-pattern>" << endl;
    return -1;
  }
  
  string loss("1");
  ReceiverStateMachine sm(atoi(argv[1]), argv[2]);
  if(sm.openSocket() < 0) {
    return -1;
  }

  vector<char>* recv = sm.receive();

  vector<char>::iterator it = recv->begin();
  for(; it != recv->end(); ++it) {
    cout << *it;
  }

  cout << endl;
  delete recv;
  return 0;

/*
  UDPSocket* socket;

  try {
    socket = new UDPSocket(LOCAL_PORT);
  } catch (exception& e) {
    cout << e.what() << endl;
    return -1;
  }

  string source_address;
  unsigned short source_port;
  char buffer[101];
  int buffer_len = 101;
  int read_bytes;

  try {
    read_bytes = socket->recvFrom((void*) buffer, buffer_len, source_address,
          source_port);
  } catch (exception& e) {
    cout << e.what() << endl;
    return -1;
  }

  cout << "Read " << read_bytes << " bytes from " << source_address << 
    ":" << source_port << endl;
  cout << buffer << endl;
  
  delete socket;
  return 0;
*/
}

