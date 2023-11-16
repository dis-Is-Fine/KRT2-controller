#include "krt2-ctrl.h"

#ifndef DEBUG
#warning This library is not fully finished, bugs may be present
#endif
#ifdef TEST
#warning Library is running in test mode
#endif

struct KRT2_frequency* _frequency;
struct KRT2_communication* _communication;
char* _status;
char* _error;
static char ACK = 0x06;
static char NAK = 0x15;

int new_active_frequency(); int new_stby_frequency(); int new_communication_cfg();
int new_PTT(); int new_intercom_vol(); int new_ext_audio_vol(); int new_sidetone();
int set_active_frequency(unsigned char frequency, unsigned char channel, char name[9]);
int set_stby_frequency(unsigned char frequency, unsigned char channel, char name[9]);
int set_new_communication_cfg(char volume, char squelch, char intercom_squelch);
int set_PTT(char PTT); int set_intercom_vol(char volume);
int set_ext_audio_vol(char ext_volume); int set_sidetone(char sidetone);
int send_data(char* buf, int size, int max_attempts);

/* Initalizes KRT2 radio
  assigns krt status structures and variables
  returns 0 on success, -1 on fault
  and -2 on timeout or garbage data recieved*/
int krt_init(char* file, struct KRT2_frequency* freq,
            struct KRT2_communication* comm, char* status, char* error) {

    
    _frequency = freq;
    _communication = comm;
    _status = status;
    _error = error;
    
    CHECK(serial_init(file, B9600));
    CHECK(non_canonical_set(0, 1));
    
    char buf = 0;
    char n_bytes;
    int attempts = 0;

    #ifndef TEST
    do {
        CHECKa(serial_readB(&buf), &n_bytes);
    
        /* KRT2 radio sends 'S' repeatedly while waiting for connection */
        if(buf != 'S') {
            continue;
        }
        /* respond back with any ASCII character */    
        serial_write("C", 1);
        break;
    } while (attempts++ < 64);

    if(buf != 'S') {
        if(n_bytes > 0) {PRINT_ERROR_MSG("Garbage data recieved while connecting to KRT2"); return -2;}
        PRINT_ERROR_MSG("Timeout while connecting to KRT2"); return -1;
    }
    #endif

    return 0;
}

int krt_check() {

    char buf[1];
    int n_bytes;

    do {
        non_canonical_set(0, 0);

        n_bytes = serial_readB(buf);

        if(n_bytes < 0) {
            return -1;
        }

    } while(buf[0] != 0x02 && n_bytes != 0); /* Each KRT2 transmition starts with STX (0x02) */

    if(n_bytes == 0) return 0;

    n_bytes = serial_readB(buf);
    if(n_bytes <= 0) {
        return -1;
    }

    switch(buf[0]) {
        case _ACT_FREQ: new_active_frequency(); break;
        case _STBY_FREQ: new_stby_frequency(); break;
        case _COMM_CFG: new_communication_cfg(); break;
        case _PTT: new_PTT(); break;
        case _IC_VOL: new_intercom_vol(); break;
        case _EXT_VOL: new_ext_audio_vol(); break;
        case _SIDETONE: new_sidetone(); break;
        case _SPACING833: _communication->spacing = _SPACING833; break;
        case _SPACING25: _communication->spacing = _SPACING25; break;
        case _STATUS_BAT: *_status |= _MASK_BAT; break;
        case _STATUS_BAT_CNCL: *_status &= ~_MASK_BAT; break;
        case _STATUS_RX: *_status |= _MASK_RX; break;
        case _STATUS_RX_CNCL: *_status &= ~_MASK_RX; break;
        case _STATUS_TX: *_status |= _MASK_TX; break;
        case _STATUS_RX_TXCNCL: *_status &= ~(_MASK_RX | _MASK_TX | _MASK_DUAL_RX); break;
        case _STATUS_TIMEOUT: *_status |= _MASK_TIMEOUT; break;
        case _STATUS_DUAL_ON: *_status |= _MASK_DUAL_ON; break;
        case _STATUS_DUAL_OFF: *_status &= ~_MASK_DUAL_ON; break;
        case _ERROR_ADC: *_error |= _MASK_ERROR_ADC; break;
        case _ERROR_ANTENNA: *_error |= _MASK_ERROR_ANTENNA; break;
        case _ERROR_FPAA: *_error |= _MASK_ERROR_FPAA ; break;
        case _ERROR_FREQ_SYNTH: *_error |= _MASK_ERROR_FREQ_SYNTH; break;
        case _ERROR_PLL: *_error |= _MASK_ERROR_PLL; break;
        case _ERROR_INPUT_BLK: *_error |= _MASK_ERROR_INPUT_BLK; break;
        case _ERROR_I2C_BUS: *_error |= _MASK_ERROR_IC2_BUS; break;
        case _ERROR_D10_DIODE: *_error |= _MASK_ERROR_D10_DIODE; break;
        case _ERROR_CLEAR : *_error = 0; break;
        default: serial_write(&NAK, 1);
    }

    return 0;
}

int new_active_frequency(){
    char buf[11];
    non_canonical_set(11, 10);
    serial_read(buf, 11);
    if((buf[0] ^ buf[1]) != buf [10]) {
        serial_write(&NAK, 1);
        return -1;
    }
    _frequency->active_frequency = buf[0];
    _frequency->active_channel = buf[1];
    for(int i = 0; i < 8; i++) {
        _frequency->active_name[i] = buf[i+2];
    }
    _frequency->active_name[8] = 0;
    serial_write(&ACK, 1);
    return 0;
}

int new_stby_frequency(){
    char buf[11];
    non_canonical_set(11, 10);
    serial_read(buf, 11);
    if((buf[0] ^ buf[1]) != buf [10]) {
        serial_write(&NAK, 1);
        return -1;
    }
    _frequency->stby_frequency = buf[0];
    _frequency->stby_channel = buf[1];
    for(int i = 0; i < 8; i++) {
        _frequency->stby_name[i] = buf[i+2];
    }
    _frequency->stby_name[8] = 0;
    serial_write(&ACK, 1);
    return 0;
}

int new_communication_cfg(){
    char buf[4];
    non_canonical_set(4, 10);
    serial_read(buf, 4);
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
    serial_read(&buf, 1);
    _communication->PTT = buf;
    serial_write(&ACK, 1);
    return 0;
}

int new_intercom_vol(){
    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1);
    _communication->intercom_volume = buf;
    serial_write(&ACK, 1);
    return 0;
}

int new_ext_audio_vol(){
    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1);
    _communication->external_input = buf;
    serial_write(&ACK, 1);
    return 0;
}

int new_sidetone(){
    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1);
    _communication->sidetone = buf;
    serial_write(&ACK, 1);
    return 0;
}

int set_active_frequency(unsigned char frequency, unsigned char channel, char name[9]){
    char buf[13];

    if(frequency < 118) frequency = 118;
    if(frequency > 136) frequency = 136;
    if(channel < 0) channel = 0;
    if(channel > 198) channel = 198;

    buf[0] = 0x02;
    buf[1] = _ACT_FREQ;
    buf[2] = frequency;
    buf[3] = channel;
    strcpy(buf+4, name);
    buf[12] = buf[2] ^ buf[3];

    CHECKnp(send_data(buf, 13, 3));

    _frequency->active_frequency = frequency;
    _frequency->active_channel = channel;
    strcpy(_frequency->active_name, name);
    return 0;    
}

int set_stby_frequency(unsigned char frequency, unsigned char channel, char name[9]){
    char buf[13];

    if(frequency < 118) frequency = 118;
    if(frequency > 136) frequency = 136;
    if(channel < 0) channel = 0;
    if(channel > 198) channel = 198;

    buf[0] = 0x02;
    buf[1] = _STBY_FREQ;
    buf[2] = frequency;
    buf[3] = channel;
    strcpy(buf+4, name);
    buf[12] = buf[2] ^ buf[3];

    if(send_data(buf, 13, 3) < 0) return -1;

    _frequency->stby_frequency = frequency;
    _frequency->stby_channel = channel;
    strcpy(_frequency->stby_name, name);
    return 0;    
}

int set_new_communication_cfg(char volume, char squelch, char intercom_squelch){
    char buf[6];

    buf[0] = 0x02;
    buf[1] = _COMM_CFG;
    buf[2] = volume;
    buf[3] = squelch;
    buf[4] = intercom_squelch;
    buf[5] = squelch ^ intercom_squelch;

    if(send_data(buf, 6, 3) < 0) return -1;

    _communication->volume = volume;
    _communication->squelch = squelch;
    _communication->intercom_squelch = intercom_squelch;

    return 0;
}

int set_PTT(char PTT){
    char buf[3];
    
    buf[0] = 0x02;
    buf[1] = _PTT;
    buf[2] = PTT;

    CHECKnp(send_data(buf, 3, 3));

    _communication->PTT = PTT;

    return 0;
}

int set_intercom_vol(char volume) {
    char buf[3];

    buf[0] = 0x02;
    buf[1] = _IC_VOL;
    buf[2] = volume;

    if(send_data(buf, 3, 3) < 0) return -1;

    _communication->intercom_volume = volume;

    return 0;
}

int set_ext_audio_vol(char ext_volume) {

  char buf[3];

  buf[0] = 0x02; 
  buf[1] = _EXT_VOL;
  buf[2] = ext_volume;

  if(send_data(buf, 3, 3) < 0) return -1;

  _communication->external_input = ext_volume;

  return 0;
}

int set_sidetone(char sidetone) {

  char buf[3];

  buf[0] = 0x02;
  buf[1] = _SIDETONE;
  buf[2] = sidetone;

  if(send_data(buf, 3, 3) < 0) return -1;

  _communication->sidetone = sidetone;

  return 0; 
}

int set_spacing(char spacing) {
    if(spacing != _SPACING25 && spacing != _SPACING833) {
        PRINT_ERROR_MSG("Invalid spacing value provided");
        return -1;
    }

    char buf[2];

    buf[0] = 0x02;
    buf[1] = spacing;

    CHECKnp(send_data(buf, 2, 0));
    
    _communication->spacing=spacing;

    return 0;
}

unsigned char get_channel(int khz) {
    if(khz > 995) return 198;
    if(khz < 0) return 0;
    if (khz % 25 == 20) {
        khz = khz - 5;
    }
    int channel = khz / 5;
    return channel;
}

int get_khz(unsigned char channel) {
    if(channel < 0) return 0;
    if(channel > 198) return 990;
    return channel*5;
}

char* get_spacing_str(char spacing) {
    if(spacing == _SPACING25) {
        return "25";
    }
    return "8.33";
}

/* Sends 'size' bytes from 'buf' to KRT2 radio
  If ACK response is required 'max_attempts' should be set
  to how many attempts of writes to make
  If ACK response is not required set 'max_attempts' to 0 */
int send_data(char* buf, int size, int max_attempts) {
    char response = 0;
    int attempts = 0;
    
    #ifndef TEST
    if(max_attempts > 0){
        LOG("Setting non canonical reads to 0|10");
        CHECK(non_canonical_set(0,10));
        LOG("Sending data to KRT2 with acknowledgement check");
        while(response != ACK && attempts++ < max_attempts) {
            serial_write(buf, size);
            serial_readB(&response);
        }
        if(attempts >= max_attempts) {
            #ifdef DEBUG
                PRINT_ERROR_MSG("Failed to write to KRT2 (no acknowledgement)");
            #endif
            LOG("Failed to write to KRT2 (no acknowledgement)");
            return -1;
        }
        return 0;
    }
    #endif

    LOG("Sending data to KRT2 without acknowledgement check");
    CHECK(serial_write(buf, size));

    return 0;
}