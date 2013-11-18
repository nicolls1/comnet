#include "receiver_state_machine.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string.h>
#include <utility>

#define FILE_BUF_LEN 512

using namespace std;

ReceiverStateMachine::ReceiverStateMachine(int port, string file) : recvPort_(port),
  socket_(NULL), ackedPosition_(0), fin_(false), RSN_(0), periodic_(false),
  lossPeriod_(0), specific_(false), specificPosition_(0) {
  recvData_ = new vector<char>();

  cout << "file: " << file << " len: " << file.size() << endl;
  if(openLossPatern(file) == -1) {
    cout << "Failed to load loss file" << endl;
  }
}

int ReceiverStateMachine::openLossPatern(string file) {
  char data[FILE_BUF_LEN];
  char* pch;
  int temp;
  ifstream ifs;

  ifs.open(file, std::ifstream::in);

  ifs.getline(data, FILE_BUF_LEN);
  while(strlen(data) == 0) {
    ifs.getline(data, FILE_BUF_LEN);
  }

  pch = strtok(data," ");
  temp = atoi(pch);

  switch(temp) {
  case 0:
    cout << "No loss patern" << endl;
    return 0;
    break;
  case 1:
    if(pch != NULL) {
      pch = strtok (NULL, " ");
      lossPeriod_ = atoi(pch);
      if(lossPeriod_ < 2) { // losing every packet is not allowed
        return -1;
      }
      periodic_ = true;
      cout << "Pattern loss of " << lossPeriod_ << endl;
    } else {
      return -1;
    }
    return 0;
    break;
  case 2:
    cout << "Specific loss with ";
    while(pch != NULL) {
      pch = strtok (NULL, " ");
      if(atoi(pch) > 0) {
        specificPackets_.push_back(atoi(pch));
      } else {
        cout << endl << "Invalid loss position" << endl;
      }
      cout << pch << " ";
    }
    cout << "being lost";
    if(specificPackets_.size() == 0) {
      return -1;
    }
    specific_ = true;
    return 0;
    break;
  default:
    return -1;
    break;
  }
/*
  while (pch != NULL)
  {
    printf ("%s\n",pch);
    pch = strtok (NULL, " ");
  }
*/
  return -1;
}

int ReceiverStateMachine::openSocket() {
  try {
    socket_ = new UDPSocket(recvPort_);
  } catch (exception& e) {
    cout << e.what() << endl;
    return -1;
  }
  return 0;
}

vector<char>* ReceiverStateMachine::receive() {
  if(socket_ == NULL) {
    cout << "Socket not open" << endl;
    return NULL;
  }

  while(!fin_) {
    //cout << "waiting for packet..." << endl;
    cout << "======================================================" << endl;
    Packet packet = {0};
    int bytes_read = receivePacket(packet);

    if(bytes_read == -1) {
      continue;
    }
    if(bytes_read == -2) {
      return NULL;
    }

    //cout << "received packet, size = " << bytes_read << endl;
    cout << "seq num: " << packet.seq_num << endl;
    cout << "data len" << packet.data_len << endl;
    if(packet.FIN) {
      cout << "fin: " << packet.FIN << endl;
    }
    cout << "data: " << packet.data << endl;

    //char* buf = reinterpret_cast<char*>(&packet);

    //for(int i=0; i < sizeof(Packet); i++) {
    //  cout << hex << (int) buf[i] << endl;
    //}

    // seq num is too large
    if(packet.seq_num > (ackedPosition_ + 1)) {
      cout << "packet received was early" << endl;
      storeEarlyData(packet.seq_num, packet.data, packet.data_len, packet.FIN);
      sendAck(packet.seq_num, ackedPosition_);
      continue;
    }

    // seq num is too small
    if(packet.seq_num < (ackedPosition_ + 1)) {
      cout << "packet received was already received" << endl;
      sendAck(packet.seq_num, ackedPosition_);
      continue;
    }

    cout << "packet was the next packet we expected" << endl;

    // seq num is what is expected
    processPacket(packet, bytes_read);

    // check if we have early data that is now valid
    checkEarlyData();
    
    cout << "sending ack for position = " << ackedPosition_+1 << endl;
    sendAck(packet.seq_num, ackedPosition_);
  }

  return recvData_;
}

void ReceiverStateMachine::processPacket(Packet packet, int bytes_read) {
  if(packet.FIN) {
    fin_ = true;
  }

  saveData(packet.data, packet.data_len);

  ackedPosition_ += packet.data_len;
}

void ReceiverStateMachine::saveData(const char* data, int length) {
    for(int i = 0; i < length; ++i) {
      recvData_->push_back(data[i]);
    }
}

int ReceiverStateMachine::receivePacket(Packet& recv) {
  string source_address;
  unsigned short source_port;
  int bytes_read;

  try {
    bytes_read = socket_->recvFrom((void*)&recv, sizeof(Packet), 
        source_address, source_port);
  } catch(exception& e) {
    cout << e.what() << endl;
    return -2;
  }

  // start with rsn of zero so we pre-increment so first is one
  ++RSN_;
  cout << "RSN: " << RSN_ << endl;

  if(periodic_) {
    if((RSN_ == 1) || (RSN_ % (lossPeriod_+1) == 0)) {
      // drop packet
      cout << "Dropping a packet!" << endl;
      return -1;
    }
  }
  
  if(specific_) {
    if(RSN_ == specificPackets_[specificPosition_]) {
      //drop packet
      ++specificPosition_;
      return -1;
    }
  }

  
  if(ackedPosition_ == 0) {
    sourceAddress_ = source_address;
    sourcePort_ = source_port;
  }

  return bytes_read;
}

bool ReceiverStateMachine::sendAck(int seq_num, int ack_num) {
  Packet p = {0};

  p.seq_num = seq_num;
  p.ack_num = ack_num+1;
  p.ACK = true;
  p.FIN = fin_;

  try {
    socket_->sendTo((void*)&p, sizeof(Packet), sourceAddress_, sourcePort_);
  } catch(exception& e) {
    cout << e.what() << endl;
    return false;
  }
  return true;
}

void ReceiverStateMachine::storeEarlyData(int num, char* data, int len, bool fin) {
  cout << "Storing early data at " << num << endl;
  string s_data(data);
  s_data.resize(len);
  buffer_[num] = make_pair(s_data, fin);
}

void ReceiverStateMachine::checkEarlyData() {
  map<int, pair<string, bool> >::iterator it = buffer_.begin();
  cout << "Looking at " << ackedPosition_ << endl;
  while(true) {
    if((it = buffer_.find(ackedPosition_+1)) != buffer_.end()) {
      cout << "Found buffered data" << endl;
      cout << "data: " << it->second.first << endl;
      cout << "ackedPosition: " << ackedPosition_ << endl;
      saveData(it->second.first.c_str(), (int)it->second.first.size());
      ackedPosition_ += it->second.first.size();
      if(it->second.second) {
        fin_ = true;
      }
      buffer_.erase(it);
    } else {
      break;
    }
  }
}

