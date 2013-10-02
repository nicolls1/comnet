#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>

/**
 * This function accepts a socket FD and a ptr to a destination
 * buffer. It will receive from the socket until the EOL byte
 * sequence is seen. The EOL bytes are read from the socket, but
 * the destination buffer is terminated before these bytes.
 * Returns the size of the read line (without EOL bytes).
 */
int recv_line(int socket_fd, char* dest_buffer);

/**
 * Receives the entire stream. Returns size or -1 on error
 */
int recv_stream(int socket_fd, char* dest_buffer, int size);

/**
 * This function accepts a socket fd and a ptr to the null terminated
 * string to send. The function will make sure all the bytes of the 
 * string are sent. Returns 1 on success and 0 on failure.
 */
int send_string(int socket_fd, const char* buf);

/**
 * Takes a socket and a file ptr and sents the file through the socket.
 * Return 1 on success 0 on failure.
 */
int send_file(int socket_fd, char* request_file);

/**
 * This function accepts an open file descriptor and returns
 * the size of the associated file. Returns -1 on failure.
 */
int get_file_size(char* file_str);


#endif
