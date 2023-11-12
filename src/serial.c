#include "serial.h"

int serial_port;
struct termios tty;

int serial_init(char* file, int baud) {

    serial_port = open(file, O_RDWR | O_NOCTTY);

    if (serial_port < 0) {
        perror("Error while opening serial port file");
        return -1;
    }
    
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_lflag &= ~(ECHO | ECHOE);
    tty.c_lflag &= ~ICANON;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag &= ~CRTSCTS;
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VTIME] = 30;
    tty.c_cc[VMIN] = 0;

    if(cfsetspeed(&tty, baud) < 0) {
        perror("Error while setting baud rate");
        return -1;
    }

    if(tcsetattr(serial_port, TCSANOW, &tty) < 0) {
        perror("Error while saving attributes");
        return -1;
    }

    return 0;
}

int non_canonical_set(int min_bytes, int max_time) {
    tty.c_cc[VTIME] = max_time;
    tty.c_cc[VMIN] = min_bytes;
    
    if(tcsetattr(serial_port, TCSANOW, &tty) < 0) {
        perror("Error while saving attributes");
        return -1;
    }

    return 0;
}

int serial_write(char* msg, int size) {

    int n_bytes = write(serial_port, msg, size);
    return n_bytes;

}

int serial_read(char* buf, int buf_size) {

    int n_bytes = read(serial_port, buf, buf_size);
    if(n_bytes < 0) perror("Error while reading from serial port");
    return n_bytes;

}

int serial_readB(char* buf) {
    return(serial_read(buf, 1));
}

void serial_end() {

    close(serial_port);

}