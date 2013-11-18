#ifndef RECEIVER_STATE_MACHINE_H
#define RECEIVER_STATE_MACHINE_H

#include "packet.h"
#include "udp_socket.h"

#include <string>
#include <vector>
#include <utility>
#include <map>

class ReceiverStateMachine {
  public:
    ReceiverStateMachine(int port, std::string file);
    int openSocket();

    /**
     * buffer to place the bytes read
     * returns number of bytes read and -1 on error
     */
    vector<char>* receive();

  private:
    std::string sourceAddress_;
    int sourcePort_;
    int recvPort_;
    UDPSocket* socket_;

    int ackedPosition_;
    bool fin_;

    std::vector<char>* recvData_;

    int RSN_;
    bool periodic_;
    int lossPeriod_;
    bool specific_;
    std::vector<int> specificPackets_;
    int specificPosition_;

    std::map<int, std::pair<std::string, bool> > buffer_;

    int openLossPatern(std::string file);
    void processPacket(Packet packet, int bytes_read);
    void saveData(const char* data, int length);
    int receivePacket(Packet& recv);
    bool sendAck(int seq_num, int ack_num);
    void storeEarlyData(int num, char* data, int len, bool fin);
    void checkEarlyData();
};

#endif
