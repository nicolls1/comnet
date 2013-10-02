#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctime>
#include <chrono>
#include "network.h"

#define BUFSIZE 1024
#define EOL "\r\n"
#define EOL_SIZE 2

using namespace std;

int main(int argc, char* argv[]) {
    int port;
    if((argc != 4) || ((port = atoi(argv[2])) <= 1000 || (strlen(argv[1]) > 1000))) {
        cout << "Invalid args expect a port greater than 1000 and file name less than 1000 bytes" << endl;
        exit(EXIT_FAILURE);
    }

    int socket_fd;

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in remoteSocketInfo;
    struct hostent* hPtr;

    bzero(&remoteSocketInfo, sizeof(sockaddr_in));

    if((hPtr = gethostbyname(argv[1])) == NULL) {
        close(socket_fd);
        cerr << "system DNS name resolution not configured properly." << endl;
        cerr << "Error Number: " << ECONNREFUSED << endl;
        exit(EXIT_FAILURE);
    }

    // load system information for remote socket server into socket data structures

    memcpy((char*) &remoteSocketInfo.sin_addr, hPtr->h_addr, hPtr->h_length);
    remoteSocketInfo.sin_family = AF_INET;
    remoteSocketInfo.sin_port = htons((u_short)port);

    if(connect(socket_fd, (struct sockaddr *)&remoteSocketInfo, sizeof(sockaddr_in)) < 0) {
        cout << "unable to connect" << endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // build the string to send
    int bytes_read;
    char buf[BUFSIZE] = "GET ";
    unsigned long time_1, time_2;
    
    strncat(buf, argv[3], strlen(argv[3]));
    strncat(buf, EOL, EOL_SIZE);

    send_string(socket_fd, buf);

    time_1 = chrono::system_clock::now().time_since_epoch() /
             chrono::milliseconds(1);

    cout << buf << endl;
   
    int count = 0, content_length, read_length, response_code;
    char* content_ptr;
    while(1) {
        if((bytes_read = recv_line(socket_fd, buf)) == 1) {
            cout << buf << endl;
            break;
        }
        ++count;

        cout << buf << endl;

        if(count == 1) {
            response_code = atoi(strchr(buf,' ')+1);
            if(response_code ==400) {
                cout << "Bad Request" << endl;
                exit(EXIT_FAILURE);
            } else if(response_code == 404) {
                cout << "File Not Fond" << endl;
                exit(EXIT_FAILURE);
            }
        } else if(count == 2) {
            content_length = atoi(strchr(buf, ':')+1);
        }
    }

    cout << "content_length: " << content_length << endl;

    if((content_ptr = (char*) malloc(content_length)) == NULL) {
        cout << "failed allocating space" << endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if((read_length = recv_stream(socket_fd, content_ptr, content_length)) < 0) {
        cout << "failed to receive file" << endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    cout << content_ptr << endl;

    time_2 = chrono::system_clock::now().time_since_epoch() /
             chrono::milliseconds(1);

    cout << "Seconds since send=" << (double(time_2 - time_1))/1000 << endl;

    /* 
    FILE* store_file = fopen(argv[3], "w");
    fwrite(content_ptr, sizeof(char), read_length, store_file);
    fclose(store_file);
    */

    close(socket_fd);

    return 0;
}
