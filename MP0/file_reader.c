#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    int fd;
    unsigned char ptr[200];

    fd = open(argv[1], O_RDONLY, 0);

    read(fd, ptr, 10);

    printf("File: %s", ptr);

    close(fd);
}
