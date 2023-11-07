#include "/usr/include/stdio.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "/usr/include/termios.h"

void serial_init(char* file, int baud_rate) {
    
    struct termios tty;

    int serial_port = open(file, O_RDWR);

    if (serial_port < 0) {
        printf("Error %i from open:%s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;


}