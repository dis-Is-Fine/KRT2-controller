#include "krt2-ctrl.h"

struct KRT2_frequency* frequency;
struct KRT2_communication* communication;

int krt_init(char* file, struct KRT2_frequency* frequency,
            struct KRT2_communication* communication) {
    
    if(serial_init(file, B9600) < 0) {
        return -1;
    }
    
    char buf[1];
    int n_bytes = serial_readB(buf);

    /* KRT2 radio sends 'S' repeatedly while waiting for connection */
    if(*buf != 'S') {
        return -1;
    }
    /* respond back with any ASCII character */    
    serial_write("C", 1);
    
    return 0;
}

int krt_check() {

    char buf[20];
    int n_bytes;

    do {
        n_bytes = serial_readB(buf);

        if(n_bytes < 0) {
            fprintf(stderr, "Error while reading from serial port");
            return -1;
        }

        if(n_bytes == 0) {
            return 0;
        }

    } while(buf[0] != 0x02); /* Each KRT2 transmition starts with STX (0x02) */
    
    n_bytes = serial_readB(buf);
    if(n_bytes <= 0) {
        fprintf(stderr, "Error while reading from serial port");
        return -1;
    }

    if(buf[0] != 0x02) {
        fprintf(stderr, "Invalid data recieved");
        return -1;
    }

    return 0;

}
