#ifndef SENDER_STATE_MACHINE_H
#define SENDER_STATE_MACHINE_H

#include <fstream>
#include <string>
#include <map>
#include <stdint.h>

#include "packet.h"
#include "udp_socket.h"

class SenderStateMachine {
  public:
    SenderStateMachine(string ip, int local_port, int receive_port);
    ~SenderStateMachine();
    int openSocket();
    bool transmit(string file_name);

  private:
    int localPort_;
    string recvIP_;
    int recvPort_;

    //std::ifstream* message_;

    int cwnd_; // window size, in MMS
    int notAcked_; // number of sent messages and not acked
    int maxNotAcked_;
    int ssthresh_;
    int dupAckCount_;
    uint32_t ackedPosition_; // postion of last acked bytes
    uint32_t filePosition_;
    int caAckCount_;
    bool ackedFin_;

    double alpha_;
    double beta_;
    unsigned long estimatedRTT_;
    unsigned long devRTT_;
    unsigned long timeoutInterval_;
    std::map<int, unsigned long> sendTimes_;

    enum State {
      SLOW_START,
      CONGESTION_AVOIDANCE,
      FAST_RECOVERY
    };

    State state_;
    UDPSocket* socket_;

    std::ofstream trace_;
    std::ofstream cwndFile_;
    std::ifstream* message_;
    std::string fileName_;

    /**
     * data to send in udp packet
     * returns number of bytes sent
     */
    int sendData(char* data, int len);
    bool retransmitMissingSegment();
    int receivePacket(Packet& recv);

    /**
     * return 1 on finished
     *        0 for normal exec
     *        -1 on error
     */
    int ssReceive();
    int caReceive();
    int frReceive();

    void processTimeout();
    bool finalAck(Packet recv);
    bool dupAck(Packet recv);
    bool newAck(Packet recv);

    void updateRTT(unsigned long seq_num);
    unsigned long getTime();
};

#endif
