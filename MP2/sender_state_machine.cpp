#include "sender_state_machine.h"

#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>

#define MMSBYTES 100

using namespace std;

SenderStateMachine::SenderStateMachine(string ip, int local_port, 
    int receive_port) : localPort_(local_port), recvIP_(ip), 
  recvPort_(receive_port), cwnd_(1), notAcked_(0), maxNotAcked_(25), 
  ssthresh_(1000), dupAckCount_(0), ackedPosition_(1), filePosition_(0), 
  ackedFin_(false), alpha_(.125), beta_(.25), estimatedRTT_(100000), 
  devRTT_(20000), timeoutInterval_(100000), state_(SLOW_START), 
  socket_(NULL) { }

SenderStateMachine::~SenderStateMachine() {
  delete message_;
}

int SenderStateMachine::openSocket() {
  try {
    socket_ = new UDPSocket(localPort_);
  } catch (exception& e) {
    cout << e.what() << endl;
    return -1;
  }
  return 0;

  socket_->setTimeout(0, timeoutInterval_); // starting timeout
}

bool SenderStateMachine::transmit(string file_name) {
  if(socket_ == NULL) {
    cout << "socket not open" << endl;
    return false;
  }
  
  trace_.open("trace", std::ifstream::out | std::ifstream::trunc);
  cwndFile_.open("cwnd", std::ifstream::out | std::ifstream::trunc);
  cwndFile_ << getTime() << " " << (double)(cwnd_ * MMSBYTES) << endl;
   
  message_ = new ifstream();
  message_->open(file_name, std::ifstream::in);
  fileName_ = file_name;

  while(true) {
    // transmit as much as we can
    while((*message_) && (notAcked_ < cwnd_) && (notAcked_ < maxNotAcked_)) {
      cout << "======================================================" << endl;
      cout << "cwnd:" << cwnd_ << endl;
      char packet_data[MMSBYTES] = {0};
      int bytes_sent;
      //cout << "MessagePosition: " << message_->tellg() << endl;
      message_->read(packet_data, MMSBYTES);
      //cout << "MessagePosition: " << message_->tellg() << endl;
      if(*message_) {
        if((bytes_sent = sendData(packet_data, MMSBYTES)) < 0) {
          cout << "Error sending File" << endl;
          return false;
        }
      } else {
        if((bytes_sent = sendData(packet_data, message_->gcount())) < 0) {
          cout << "Error sending File" << endl;
          return false;
        }
      }
      filePosition_ += bytes_sent;
      //cout << "BytesSent: " << bytes_sent << endl;
      //cout << "FilePosition: " << filePosition_ << endl;
    }

    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    //cout << "finished sending, waiting for an ack" << endl;
    socket_->setTimeout(0, 100000); // starting timeout

    int status = -1;
    // wait for replies
    switch(state_) {
    case SLOW_START:
      cout << "Slow Start" << endl;
      status = ssReceive();
      break;
    case CONGESTION_AVOIDANCE:
      cout << "Congestion Avoidance" << endl;
      status = caReceive();
      break;
    case FAST_RECOVERY:
      cout << "Fast Recovery" << endl;
      status = frReceive();
      break;
    default:
      cout << "Error in state" << endl;
      break;
    }

    switch(status) {
    case 1:
      return true;
      break;
    case 0:
      // do nothing, worked correctly
      break;
    case -1:
      return false;
      break;
    default:
      break;
    }
  }
}

int SenderStateMachine::sendData(char* data, int len) {
  Packet p = {0};

  p.seq_num = filePosition_+1;
  p.data_len = len;
  p.ACK = false;
  p.FIN = (bool)!(*message_);
  for(unsigned int i = 0; i < strlen(data); ++i) {
    p.data[i] = data[i];
  }

  cout << "seq_num: " << p.seq_num << endl;
  //cout << "data_len: " << p.data_len << endl;
  if(p.FIN) {
    cout << "FIN: " << p.FIN << endl;
  }
  //cout << "data: " << data << endl;

  //char* buf = reinterpret_cast<char*>(&p);
   
  //for(int i=0; i < sizeof(Packet); i++) {
    //cout << hex << (int) buf[i] << dec << endl;
  //}

  sendTimes_[p.seq_num] = getTime();

  try {
    socket_->sendTo((void*)&p, sizeof(p), recvIP_, recvPort_);
  } catch (exception& e) {
    cout << e.what() << endl;
    return -1;
  }
  ++notAcked_;
  return strlen(data)-2;
}

bool SenderStateMachine::retransmitMissingSegment() {
  cout << "retransmitMissingSegment" << endl;
  int start_position = message_->tellg();
  if(start_position == -1) {
    delete message_;
    message_ = new ifstream();
    message_->open(fileName_, std::ifstream::in);
    message_->seekg(filePosition_);
    start_position = filePosition_;
  }

  cout << "AckedPosition: " << ackedPosition_ << endl;
  cout << "StartPosition: " << start_position << endl;
  cout << "filePosition: " << filePosition_ << endl;
  message_->seekg(ackedPosition_-1);
  filePosition_ = ackedPosition_-1;

  char packet_data[MMSBYTES];
  int bytes_sent;
  message_->read(packet_data, MMSBYTES);
  cout << "Retrans Data: " << packet_data << endl;
  if(*message_) {
    if((bytes_sent = sendData(packet_data, MMSBYTES)) < 0) {
      cout << "Error sending File" << endl;
      return false;
    }
  } else {
    if((bytes_sent = sendData(packet_data, message_->gcount())) < 0) {
      cout << "Error sending File" << endl;
      return false;
    }
  }

  message_->seekg(start_position);
  filePosition_ = start_position;//-1;
  cout << "AckedPosition: " << ackedPosition_ << endl;
  cout << "StartPosition: " << message_->tellg() << endl;
  cout << "filePosition: " << filePosition_ << endl;
  return true;
}

int SenderStateMachine::receivePacket(Packet& recv) {
  string source_address;
  unsigned short source_port;
  int bytes_read;
  // wait for packets to be acked
  try {
    bytes_read = socket_->recvFrom(&recv, sizeof(Packet), source_address, source_port);
  } catch(exception& e) {
    cout << e.what() << endl;
    return -1;
  }

  return bytes_read;
}

int SenderStateMachine::ssReceive() {
  Packet recv;
  int bytes_read = receivePacket(recv);

  if(bytes_read < 0) {
    processTimeout();
    return 0;
  }

  if(finalAck(recv)) {
    return 1;
  }

  if(dupAck(recv)) {
    if(dupAckCount_ == 3) {
      ssthresh_ =((int) (cwnd_/2)) * MMSBYTES;
      cwnd_ = (ssthresh_/MMSBYTES) + 3;
      cwndFile_ << getTime() << " " << cwnd_ * MMSBYTES << endl;
      notAcked_ = cwnd_-3;
      state_ = FAST_RECOVERY;
      if(!retransmitMissingSegment()) {
        return -1;
      }
    }
    return 0;
  }

  // if valid ack: increase cwnd by MMS, dupAck set to 0
  if(newAck(recv)) {
    ++cwnd_;
    cwndFile_ << getTime() << " " << cwnd_ * MMSBYTES << endl;
    if(cwnd_ * MMSBYTES >= ssthresh_) {
      caAckCount_ = 0;
      state_ = CONGESTION_AVOIDANCE;
    }
    --notAcked_;
    return 0;
  }

  // we can just ignore any other packets received
  return 0;
}

int SenderStateMachine::caReceive() {
  Packet recv;
  int bytes_read = receivePacket(recv);

  if(bytes_read < 0) {
    processTimeout();
    return 0;
  }

  if(finalAck(recv)) {
    return 1;
  }

  if(dupAck(recv)) {
    if(dupAckCount_ == 3) {
      ssthresh_ =((int) (cwnd_/2)) * MMSBYTES;
      cwnd_ = (ssthresh_/MMSBYTES) + 3;
      cwndFile_ << getTime() << " " << cwnd_ * MMSBYTES << endl;
      notAcked_ = cwnd_-3;
      state_ = FAST_RECOVERY;
      if(!retransmitMissingSegment()) {
        return -1;
      }
    }
    return 0;
  }

  if(newAck(recv)) {
    ++caAckCount_;
    if(cwnd_ == caAckCount_) {
      ++cwnd_;
      cwndFile_ << getTime() << " " << cwnd_ * MMSBYTES << endl;
      caAckCount_ = 0;
    }
    --notAcked_;
    return 0;
  }

  // we can just ignore any other packets received
  return 0;
}

int SenderStateMachine::frReceive() {
  Packet recv;
  int bytes_read = receivePacket(recv);

  if(bytes_read < 0) {
    processTimeout();
    return 0;
  }

  if(finalAck(recv)) {
    return 1;
  }

  if(dupAck(recv)) {
    ++cwnd_;
    cwndFile_ << getTime() << " " << cwnd_ * MMSBYTES << endl;
    return 0;
  }

  int temp = ackedPosition_;
  if(newAck(recv)) {
    cwnd_ = ssthresh_ / MMSBYTES;
    cwndFile_ << getTime() << " " << cwnd_ * MMSBYTES << endl;
    caAckCount_ = 0;
    state_ = CONGESTION_AVOIDANCE;
    //notAcked_ = 0;
    cout << "max i: " << (recv.ack_num-temp)/MMSBYTES << endl;
    for(unsigned int i = 0; i < (recv.ack_num-temp)/MMSBYTES; ++i) {
      --notAcked_;
    }
    return 0;
  }
  
  return 0;
}

void SenderStateMachine::processTimeout() {
  cout << "TIMEOUT" << endl;
  ssthresh_ = (cwnd_ * MMSBYTES)/2;
  cout << "ssthresh: " << ssthresh_ << endl;
  cwnd_ = 1;
  cwndFile_ << getTime() << " " << cwnd_ * MMSBYTES << endl;
  dupAckCount_ = 0;
  message_->seekg(ackedPosition_-1);
  //if(ackedPosition_ != 1) {
  filePosition_ = ackedPosition_-1;
  //}
  notAcked_=0;
}

bool SenderStateMachine::finalAck(Packet recv) {
  if(recv.FIN && recv.ACK) {
    cout << "finalAck" << endl;
    return true;
  }
  return false;
}

bool SenderStateMachine::dupAck(Packet recv) {
  if(recv.ACK && (ackedPosition_ >= recv.ack_num)) {
    cout << "dupAck, ackedPosition_: " << ackedPosition_ << endl;
    cout << "recv.ack_num: " << recv.ack_num << endl;
    ++dupAckCount_;
    return true;
  }
  return false;
}

bool SenderStateMachine::newAck(Packet recv) {
  if(recv.ACK && (ackedPosition_ < recv.ack_num)) {
    // update RTT estimate
    updateRTT((unsigned long) recv.seq_num);
    trace_ << getTime() << " " << recv.seq_num << endl;

    ackedPosition_ = recv.ack_num;
    cout << "newAck: " << ackedPosition_ << endl;
    if(message_->tellg() < ackedPosition_) {
      message_->seekg(ackedPosition_-1);
    }
    dupAckCount_ = 0;
    return true;
  }
  return false;
}

void SenderStateMachine::updateRTT(uint64_t seq_num) {
  unsigned long receive_time = getTime();
  unsigned long send_time;
  if(sendTimes_.find(seq_num) != sendTimes_.end()) {
     send_time = sendTimes_.at(seq_num);
  } else {
    cout << "Received a seq number we did not send?!?" << endl;
    return;
  }
/*
  cout << "send time = " << send_time << endl;
  cout << "receive_time = " << receive_time << endl;
  cout << "1-alpha = " << 1-alpha_ << endl;
  cout << "(1-alpha)*estimatedRTT = " << (1-alpha_)*estimatedRTT_ << endl;
  cout << "receive_time-send_time = " << (unsigned long)receive_time-send_time<< endl;
  cout << "alpha*(receive_time-send_time) = " << alpha_*(receive_time-send_time) << endl;
*/
  estimatedRTT_ = (1-alpha_)*estimatedRTT_ + alpha_*(receive_time-send_time);
  cout << "EstimatedRTT_: " << estimatedRTT_ << endl;

  if(send_time > receive_time) {
    devRTT_ = (1-beta_)*devRTT_ + beta_*(send_time - receive_time);
  } else {
    devRTT_ = (1-beta_)*devRTT_ + beta_*(receive_time - send_time);
  }
  cout << "devRTT_: " << devRTT_ << endl;

  timeoutInterval_ = estimatedRTT_ + 4 * devRTT_;
  cout << "TimeoutInterval: " << timeoutInterval_ << endl;
}

unsigned long SenderStateMachine::getTime() {
  return chrono::system_clock::now().time_since_epoch()/chrono::microseconds(1);
}

long double2long(const double d)
{
    long l = static_cast<long>((static_cast<double>(d * 200.0 + 1.0))) / 2;
      return l;
}

