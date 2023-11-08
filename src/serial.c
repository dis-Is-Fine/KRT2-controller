#include "serial.h"

int serial_port;

void serial_init(char* file) {
    
    struct termios tty;

    serial_port = open(file, O_RDWR);

    if (serial_port < 0) {
        printf("Error %i from open:%s\n", errno, strerror(errno));
    }

    tty.c_cflag = CS8 | CREAD | CLOCAL ;
    tty.c_cc[VTIME] = 100;
    tty.c_cc[VMIN] = 0;
    cfsetspeed(&tty, B9600);

    tcsetattr(serial_port, TCSANOW, &tty);
}

int serial_write(char* msg, int size) {

    int n_bytes = write(serial_port, msg, size);
    return n_bytes;

}

int serial_read(char* buf, int buf_size) {

    int n_bytes = write(serial_port, buf, buf_size);
    return n_bytes;

}

void serial_end() {

    close(serial_port);

}