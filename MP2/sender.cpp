#include "sender_state_machine.h"

#include <iostream>
#include <string>

#define LOCAL_PORT 5050
#define RECEIVE_PORT 5051

using namespace std;

int main(int argc, char* argv[]) {
  if(argc != 4) {
    cout << "Need 3 args: <filename> <receiver-domain-name> <receiver-port>" << endl;
    return -1;
  }

  std::string ip("127.0.0.1");
  SenderStateMachine sm(argv[2], LOCAL_PORT, atoi(argv[3]));
  if(sm.openSocket() < 0) {
    cout << "Failed to open socket" << endl;
    return -1;
  }

  //ifstream send_file;
  //send_file.open(argv[1], ifstream::in);
  //char buf[100];

  if(!sm.transmit(argv[1])) { 
    // error
  }

  return 0;
  
/*  
  UDPSocket* socket;

  try {
    socket = new UDPSocket(LOCAL_PORT);
  } catch (exception& e) {
    cout << e.what() << endl;
    return -1;
  }

  char test_send[] = "This is a test string\n\0";
  
  try {
    socket->sendTo((void*)test_send, strlen(test_send), "127.0.0.1", RECEIVE_PORT);
  } catch (exception& e) {
    cout << e.what() << endl;
    return -1;
  }

  delete socket;
*/
  return 0;
}
