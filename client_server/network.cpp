#include "network.h"
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <fstream>

#define EOL "\r\n"
#define EOL_SIZE 2

using namespace std;

int recv_line(int socket_fd, char* dest_buffer) {
    char* ptr;
    int eol_matched = 0;

    ptr = dest_buffer;
    while(recv(socket_fd, ptr, 1, 0) == 1) { // Read a single byte
        if(*ptr == EOL[eol_matched]) {
            ++eol_matched;
            if(eol_matched == EOL_SIZE) {
                *(ptr+1-EOL_SIZE) = '\0';
                return strlen(dest_buffer);
            }
        } else {
            eol_matched = 0;
        }
        ptr++;
    }
    return 0;
}

int recv_stream(int socket_fd, char* dest_buffer, int size) {
    if(size < 1024 * 5) {
        return recv(socket_fd, dest_buffer, size, MSG_WAITALL);
    }

    int read = 0, temp;
    while(size - read > 5 * 1024) {
        if((temp = recv(socket_fd, dest_buffer+read, 5*1024, 0)) < 0) {
            return -1;
        }
        read += temp;
    }
    
    while(read != size) {
        if((temp = recv(socket_fd, dest_buffer+read, size - read, 0)) < 0) {
            return -1;
        }
        read += temp;
    }

    return read;
}

int send_string(int socket_fd, const char* buf) {
    int sent_bytes, bytes_to_send;
    bytes_to_send = strlen(buf);
    while(bytes_to_send > 0) {
        sent_bytes = send(socket_fd, buf, bytes_to_send, 0);
        if(sent_bytes == -1) {
            return 0; // return 0 on error
        }

        bytes_to_send -= sent_bytes;
        buf += sent_bytes;
    }
    return 1; // 1 for string sent
}

int send_file(int socket_fd, char* request_file_str) {
    int length, send_count = 0;
    char* ptr;
    char prefix[128] = "Content-Length:";

    if((length = get_file_size(request_file_str)) == -1) {
        return -1;
    }

    strcat(prefix, to_string(length).c_str());
    strcat(prefix, "\r\n\n\r\n");

    if((ptr = (char *) malloc(length+1)) == NULL) {
        return 0;
    }

    // send our header
    send_count += send_string(socket_fd, "HTTP 200 OK\r\n");
    send_count += send_string(socket_fd, prefix);

    // send the page
    ifstream request_file(request_file_str, ifstream::in);
    request_file.read(ptr, length);
    ptr[length] = '\0';
    send_count += send_string(socket_fd, ptr);

    free(ptr);
    
    //check all the sends worked
    if(send_count != 3) {
        return 0;
    }
    return 1;
}

int get_file_size(char* file_str) {
    ifstream file(file_str, ifstream::in);
    file.seekg(0, ifstream::end);
    return file.tellg();
}

