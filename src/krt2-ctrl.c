#include "krt2-ctrl.h"

struct KRT2_frequency* frequency;
struct KRT2_communication* communication;
static char ACK = 0x06;
static char NAK = 0x15;

int krt_init(char* file, struct KRT2_frequency* freq,
            struct KRT2_communication* comm) {

    frequency = freq;
    communication = comm;
    
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
        } else if(n_bytes == 0) {
            fprintf(stderr, "Invalid data recieved");
            return -1;
        }

    } while(buf[0] != 0x02); /* Each KRT2 transmition starts with STX (0x02) */
    
    n_bytes = serial_readB(buf);
    if(n_bytes <= 0) {
        fprintf(stderr, "Error while reading from serial port");
        return -1;
    }

    switch(buf[0]) {
        case 'U': new_active_frequency();
        case 'R': new_stby_frequency();
    }

    return 0;
}

int new_active_frequency(){
    char buf[11];
    non_canonical_set(11, 10);
    serial_read(buf, 11, 11);
    if(buf[0] ^ buf[1] != buf [10]) {
        char msg = 0x15;
        serial_write(&NAK, 1);
        return -1;
    }
    frequency->active_frequency = buf[0];
    frequency->active_channel = buf[1];
    for(int i = 0; i < 8; i++) {
        frequency->active_name[i] = buf[i+2];
    }
    serial_write(&ACK, 1);
    return 0;
}

int new_stby_frequency(){
    char buf[11];
    non_canonical_set(11, 10);
    serial_read(buf, 11, 11);
    if(buf[0] ^ buf[1] != buf [10]) {
        char msg = 0x15;
        serial_write(&NAK, 1);
        return -1;
    }
    frequency->stby_frequency = buf[0];
    frequency->stby_channel = buf[1];
    for(int i = 0; i < 8; i++) {
        frequency->stby_name[i] = buf[i+2];
    }
    serial_write(&ACK, 1);
    return 0;
}
