#include "krt2-ctrl.h"

struct KRT2_frequency* _frequency;
struct KRT2_communication* _communication;
static char ACK = 0x06;
static char NAK = 0x15;

int krt_init(char* file, struct KRT2_frequency* freq,
            struct KRT2_communication* comm) {

    _frequency = freq;
    _communication = comm;
    
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
        case _ACT_FREQ: new_active_frequency();
        case _STBY_FREQ: new_stby_frequency();
        case _COMM_CFG: new_communication_cfg();
        case _PTT: new_PTT();
        case _IC_VOL: new_intercom_vol();
        case _EXT_VOL: new_ext_audio_vol();
        case _SIDETONE: new_sidetone();
        case _SPACING833: _communication->spacing = _SPACING833;
        case _SPACING25: _communication->spacing = _SPACING25;
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
    _frequency->active_frequency = buf[0];
    _frequency->active_channel = buf[1];
    for(int i = 0; i < 8; i++) {
        _frequency->active_name[i] = buf[i+2];
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
    _frequency->stby_frequency = buf[0];
    _frequency->stby_channel = buf[1];
    for(int i = 0; i < 8; i++) {
        _frequency->stby_name[i] = buf[i+2];
    }
    serial_write(&ACK, 1);
    return 0;
}

int new_communication_cfg(){
    char buf[4];
    non_canonical_set(4, 10);
    serial_read(buf, 4, 4);
    if(buf[1] + buf[2] != buf[3]){
        serial_write(&NAK, 1);
        return -1;
    }
    _communication->volume = buf[0];
    _communication->squelch = buf[1];
    _communication->intercom_squelch = buf[2];
    serial_write(&ACK, 1);
    return 0;
}

int new_PTT(){
    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1, 1);
    _communication->PTT = buf;
    serial_write(&ACK, 1);
    return 0;
}

int new_intercom_vol(){
    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1, 1);
    _communication->intercom_volume = buf;
    serial_write(&ACK, 1);
    return 0;
}

int new_ext_audio_vol(){
    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1, 1);
    _communication->external_input = buf;
    serial_write(&ACK, 1);
    return 0;
}

int new_sidetone(){
    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1, 1);
    _communication->sidetone = buf;
    serial_write(&ACK, 1);
    return 0;
}

int set_active_frequency(int frequency, int channel, char name[8]){
    char buf[13];
    char response = 0;
    int attempts = 0;
    non_canonical_set(1, 10);

    buf[0] = 0x02;
    buf[1] = _ACT_FREQ;
    buf[2] = frequency;
    buf[3] = channel;
    for(int i = 0; i < 8; i++){
        buf[i+4] = name[i];
    }
    buf[13] = buf[0] ^ buf[1];

    while(response != ACK && attempts++ < 3) {
        serial_write(buf, 11);
        serial_read(&response, 1, 1);
    }
    if(attempts >= 3){
        return -1;
    }
    _frequency->active_frequency = frequency;
    _frequency->active_channel = channel;
    for(int i = 0; i < 8; i++){
        _frequency->active_name[i] = name[i];
    }
    return 0;    
}

int set_stby_frequency(int frequency, int channel, char name[8]){
    char buf[13];
    char response = 0;
    int attempts = 0;
    non_canonical_set(1, 10);

    buf[0] = 0x02;
    buf[1] = _STBY_FREQ;
    buf[2] = frequency;
    buf[3] = channel;
    for(int i = 0; i < 8; i++){
        buf[i+4] = name[i];
    }
    buf[13] = buf[0] ^ buf[1];

    while(response != ACK && attempts++ < 3) {
        serial_write(buf, 11);
        serial_read(&response, 1, 1);
    }
    if(attempts >= 3){
        return -1;
    }
    _frequency->stby_frequency = frequency;
    _frequency->stby_channel = channel;
    for(int i = 0; i < 8; i++){
        _frequency->stby_name[i] = name[i];
    }
    return 0;    
}