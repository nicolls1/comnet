#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "network.h"

#define BUFSIZE 512
#define MAXHOSTNAME 256

using namespace std;

void handle_connection(int, struct sockaddr_in *);
void send_bad_request(int);
void send_not_found(int);

int main(int argc, char* argv[]) {
  int port;
  if((argc != 2) || ((port = atoi(argv[1])) <= 1000)) {
    cout << "Invalid args expect a port number greater than 1000" << endl;
    exit(EXIT_FAILURE);
  }
  
  int socket_fd;

  // create our socket
  if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in socketAddress;
  char sysHost[MAXHOSTNAME+1];
  struct hostent* hPtr;

  bzero(&socketAddress, sizeof(sockaddr_in));

  // Get system information
  
  gethostname(sysHost, MAXHOSTNAME);
  if((hPtr = gethostbyname(sysHost)) == NULL) {
    close(socket_fd);
    cerr << "System Host name misconfigured";
    exit(EXIT_FAILURE);
  }

  // Load system information into socket data structure
  
  socketAddress.sin_family = AF_INET;
  // Use any address available to the system. This is a typical configuration for a server.
  // Note that this is where the socket client and socket server differ.
  // A socket client will specify the server address to connect to.
  socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  socketAddress.sin_port = htons(port);

  // Bind the socket to a local socket address
  
  if(bind(socket_fd, (struct sockaddr*) &socketAddress, sizeof(sockaddr_in)) < 0 ) {
    close(socket_fd);
    perror("bind");
    exit(EXIT_FAILURE);
  }

  cout << "Socked bound to port " << port << endl;

  if(listen(socket_fd, 5) < 0) {
    close(socket_fd);
    perror("listen");
    exit(EXIT_FAILURE);
  }

  int socketConnection;
  struct sockaddr_in theirAddr; // the info of the connecting client will be filled here 
  socklen_t size = sizeof(sockaddr_in);
  while(1) {
    cout << "Waiting for connections..." << endl;
    if((socketConnection = accept(socket_fd, (struct sockaddr *) &theirAddr, &size)) < 0) {
      close(socket_fd);
      cout << "Accept Error" << endl;
      exit(EXIT_FAILURE);
    }

    cout << "Connection received!" << endl;
    if(!fork()) {
      close(socket_fd);
      handle_connection(socketConnection, &theirAddr);
      exit(0);
    }
  }
  close(socket_fd);
}

void handle_connection(int socket_fd, struct sockaddr_in* client_addr_ptr) {
  char buf[BUFSIZE], resource[BUFSIZE];
  char* ptr;
  int receivedBytes;

  recv_line(socket_fd, buf);

  cout << "Received: " << buf << endl;

  if((ptr = strstr(buf, "GET ")) == NULL) {
    cout << "no GET" << endl;
    send_bad_request(socket_fd);
    shutdown(socket_fd, SHUT_RDWR);
    return;
  } else if(ptr != buf) { // request should start with the GET request
    cout << "invalid text before GET" << endl;
    send_bad_request(socket_fd);
    shutdown(socket_fd, SHUT_RDWR);
    return;
  }

  // get the file location
  ptr += 4; // skip the GET and the space
  
  //strcpy(resource, WEBROOT);
  //strcat(resource, ptr);

  int result;
  if((result = send_file(socket_fd, ptr)) == 0) {
    cout << "server error" << endl;
  } else if(result < 0) {
    send_not_found(socket_fd);
  }

  shutdown(socket_fd, SHUT_RDWR);
}

/** 
* Response when we get a bad requset
*/
void send_bad_request(int socket_fd) {
  const char* bad_request = "HTTP 400 Bad Request\r\n\n\r\n";
  send_string(socket_fd, bad_request);
  cout << "sent bad request" << endl;
  shutdown(socket_fd, SHUT_RDWR);
}

/**
* Response when we can not find a page
*/
void send_not_found(int socket_fd) {
  const char* not_found = "HTTP 404 Not Found\r\n\n\r\n";
  send_string(socket_fd, not_found);
  cout << "sent bad request" << endl;
  shutdown(socket_fd, SHUT_RDWR);
}
