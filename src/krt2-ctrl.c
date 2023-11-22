#include "krt2-ctrl.h"

#warning This library is not fully finished, bugs may be present
#ifdef TEST
#warning Library is running in test mode
#endif

struct KRT2_sub_frequency {
    char frequency;
    char channel;
    char name[9];
};

struct KRT2_frequency* _frequency;
struct KRT2_sub_frequency* _act_frequency;
struct KRT2_sub_frequency* _stby_frequency;
struct KRT2_communication* _communication;
char* _status;
char* _error;
char* _file;
static char ACK = 0x06;
static char NAK = 0x15;

int new_frequency(char selector); int new_communication_cfg();
int new_PTT(); int new_intercom_vol(); int new_ext_audio_vol(); int new_sidetone();
int set_frequency(unsigned char frequency, unsigned char channel, char name[9], char selector);
int set_new_communication_cfg(char volume, char squelch, char intercom_squelch);
int set_PTT(char PTT); int set_intercom_vol(char volume);
int set_ext_audio_vol(char ext_volume); int set_sidetone(char sidetone);
int send_data(char* buf, int size, int max_attempts); int connection_check();

/* Initalizes KRT2 radio
  assigns krt status structures and variables
  returns 0 on success, -1 on fault
  and -2 on timeout or garbage data recieved*/
int krt_init(char* file, struct KRT2_frequency* freq,
            struct KRT2_communication* comm, char* status, char* error) {
    
    _frequency = freq;
    _act_frequency = (struct KRT2_sub_frequency*) freq;
    _stby_frequency = ((struct KRT2_sub_frequency*) freq + 1);
    _communication = comm;
    _status = status;
    _error = error;
    _file = file;

    CHECK(serial_init(_file, B9600));
    CHECK(non_canonical_set(0, 1));

    /* In test mode don't require conection confirmation */
    #ifdef TEST
        return 0;
    #endif

    /* In test mode this isn't executed */
    char buf = 0;
    char n_bytes;
    int attempts = 0;

    do {
        CHECKa(serial_readB(&buf), &n_bytes);

        if(n_bytes == 0) continue;
    
        /* KRT2 radio sends 'S' repeatedly while waiting for connection */
        if(buf != 'S') {
            continue;
        }
        /* respond back with any ASCII character */    
        serial_write("C", 1);
        return 0;

    } while (attempts++ < 64); /* Wise man once said it is important to know when to stop trying */

    /* If connection was unsuccessful print error message and return */

    if(n_bytes == 0) {
        #ifdef DEBUG
        PRINT_ERROR_MSG("Timeout while connecting to KRT2");
        #endif
        LOG("Timeout while connecting with KRT2");
        return -1;
    }

    #ifdef DEBUG
    PRINT_ERROR_MSG("Garbage data recieved while connecting to KRT2");
    #endif
    LOG("Garbage data received while connecting to KRT2");
    return -2;
}


/* Check data received from KRT2 and handles it*/
int krt_check() {

    char buf;
    int n_bytes;

    /* Run trough all new bytes received form KRT2
      if recived byte is 0x02 or no new bytes left to read
      on fault return fault code */
    CHECKnp(non_canonical_set(0, 0));
    do {
        n_bytes = serial_readB(&buf);

        if(n_bytes < 0) {
            return n_bytes;
        }

    } while(buf != 0x02 && n_bytes != 0); /* Each KRT2 transmition starts with STX (0x02) */

    /* Continue if buf contains 0x02*/
    if(n_bytes == 0) return 0;

    n_bytes = serial_readB(&buf);
    if(n_bytes <= 0) {
        LOG("Received STX byte without continuation of transmition or encountered faulty read after STX received");
        return -1;
    }

    LOG("Received STX byte, checking next byte");
    /* Handle data based on data recieved from KRT2
      if no valid byte is received return 0
      else call needed function */
    switch(buf) {
        case _CONNECTION_CHK: CHECKnp(serial_write("C", 1)); break;
        case _ACT_FREQ: CHECKnp(new_frequency(0)); break;
        case _STBY_FREQ: CHECKnp(new_frequency(1)); break;
        case _COMM_CFG: CHECKnp(new_communication_cfg()); break;
        case _PTT: CHECKnp(new_PTT()); break;
        case _IC_VOL: CHECKnp(new_intercom_vol()); break;
        case _EXT_VOL: CHECKnp(new_ext_audio_vol()); break;
        case _SIDETONE: CHECKnp(new_sidetone()); break;
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
        default: LOG("Byte received after STX byte is invalid"); return 0;
    }

    return 0;
}

/* Sets new frequency based on received data
  on system errors return -1
  on checksum missmatch returns -2 */
int new_frequency(char selector){
    char* log_msg[2] = {"Active frequency change requested by KRT2", "Standby frequency change requested by KRT2"};
    #pragma GCC diagnostic ignored "-Wchar-subscripts"
    LOG(log_msg[selector]);

    char buf[11];
    CHECKnp(non_canonical_set(11, 10));
    CHECKnp(serial_read(buf, 11));

    if((buf[0] ^ buf[1]) != buf [10]) {
        LOG("Checksum missmatch on frequency change request from KRT2");
        CHECKnp(serial_write(&NAK, 1));
        return -2;
    }

    struct KRT2_sub_frequency* sub_frequency;
    sub_frequency = ((selector == 0) ? _act_frequency : _stby_frequency); 

    sub_frequency->frequency = buf[0];
    sub_frequency->channel = buf[1];
    for(int i = 0; i < 8; i++) {
        sub_frequency->name[i] = buf[i+2];
    }
    sub_frequency->name[8] = 0;

    CHECKnp(serial_write(&ACK, 1));
    return 0;
}

int new_communication_cfg(){
    LOG("Communicaion configuration change requested by KRT2");
    char buf[4];
    non_canonical_set(4, 10);
    serial_read(buf, 4);
    if(buf[1] + buf[2] != buf[3]){
        LOG("Checksum missmatch on communication configuration request from KRT2");
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
    LOG("PTT change requested by KRT2");

    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1);

    _communication->PTT = buf;

    serial_write(&ACK, 1);

    return 0;
}

int new_intercom_vol(){
    LOG("Intercom volume change requested by KRT2");

    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1);

    _communication->intercom_volume = buf;

    serial_write(&ACK, 1);

    return 0;
}

int new_ext_audio_vol(){
    LOG("External audio input change requested by KRT2");

    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1);

    _communication->external_input = buf;

    serial_write(&ACK, 1);

    return 0;
}

int new_sidetone(){
    LOG("Sidetone volume change requested by KRT2");

    char buf;
    non_canonical_set(1, 10);
    serial_read(&buf, 1);

    _communication->sidetone = buf;
    
    serial_write(&ACK, 1);
    
    return 0;
}

/* Sets new frequency on KRT2 radio 
  set frequency, channel in their respective arguments
  put frequency name string of up to 8 characters in 'name'
  set 'selector' to 0 for active frequency change
  set 'selector' to 1 for standby frequency change
  returns 0 on success, -1 on fault and -2 if invalid arguments are provided */
int set_frequency(unsigned char frequency, unsigned char channel, char name[9], char selector){
    char* log_msg[2] = {"Active frequency change requested by user", "Standby frequency change requested by user"};
    LOG(log_msg[selector]);

    char buf[13];

    /* Make sure arguments have right values*/
    if(frequency < 118 || frequency > 136 || channel < 0 || channel > 198) {
        LOG("Invalid frequency/channel provided");
        return -2;
    }

    buf[0] = 0x02;
    buf[1] = _ACT_FREQ;
    buf[2] = frequency;
    buf[3] = channel;
    for(int i = 0; i < 8; i++) {
        buf[i+4] = name[i];
    }
    buf[12] = buf[2] ^ buf[3];

    CHECKnp(send_data(buf, 13, 3));

    struct KRT2_sub_frequency* sub_frequency;
    sub_frequency = ((selector == 0) ? _act_frequency : _stby_frequency);

    sub_frequency->frequency = frequency;
    sub_frequency->channel = channel;
    for(int i = 0; i < 8; i++) {
        sub_frequency->name[i] = buf[i+4];
    }
    
    LOG("Frequency change succeded");
    return 0;    
}

/* Sets new communication configuration on KRT2 radio
  (that is volume, squelch and intercom squelch)
  1 <= volume <= 20
  1 <= squelch | intercom squelch <= 10
  returns 0 on success, -1 on fault and -2 if invalid arguments are provided*/
int set_new_communication_cfg(char volume, char squelch, char intercom_squelch){
    LOG("Communication configuration change requested by user");

    char buf[6];

    /* Check if parametrs are valid */
    if(volume > 20 || volume < 1 || squelch > 10 || squelch < 1 ||
    intercom_squelch > 10 || intercom_squelch < 1){
            LOG("Invalid parameters");
            return -2;
    }

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

    LOG("Communication configuration change succeded");

    return 0;
}

/* Sets PTT value on KRT2 radio
  set 'PTT' as follows:
  0 -> pilot only
  1 -> copilot only
  2 -> both
  returns 0 on success, -1 on fault, -2 if invalid argument is provided*/
int set_PTT(char PTT){
    LOG("PTT change requested by user");

    if(PTT < 0 || PTT > 2) {
        LOG("Invalid argument provided");
        return -1;
    }

    char buf[3];
    
    buf[0] = 0x02;
    buf[1] = _PTT;
    buf[2] = PTT;

    CHECKnp(send_data(buf, 3, 3));

    _communication->PTT = PTT;

    LOG("PTT change succeded");

    return 0;
}

/* Sets new intercom volume
  1 <= volume <= 9
  returns 0 on success, -1 on fault, -2 if incorrect arguments are provided */
int set_intercom_vol(char volume) {
    LOG("Intercom volume change requested by user");

    if(volume < 1 || volume > 9) {
        LOG("Invalid argument provided");
        return -2;
    }

    char buf[3];

    buf[0] = 0x02;
    buf[1] = _IC_VOL;
    buf[2] = volume;

    if(send_data(buf, 3, 3) < 0) return -1;

    _communication->intercom_volume = volume;

    LOG("Intercom volume change succeded");

    return 0;
}

/* Sets new external audio input volume
  0 <= volume <= 9
  returns 0 on success, -1 on fault, -2 if incorrect arguments are provided */
int set_ext_audio_vol(char ext_volume) {
    LOG("External audio input volume change requested by user");

    if(ext_volume < 0 || ext_volume > 9) {
        LOG("Invalid argument provided");
        return -2;
    }

    char buf[3];

    buf[0] = 0x02; 
    buf[1] = _EXT_VOL;
    buf[2] = ext_volume;

    if(send_data(buf, 3, 3) < 0) return -1;

    _communication->external_input = ext_volume;

    LOG("External audio input volume change succeded");

    return 0;
}

/* Sets new sidetone
  1 <= sidetone <= 9
  returns 0 on success, -1 on fault, -2 if incorrect arguments are provided */
int set_sidetone(char sidetone) {
    LOG("Sidetone change requested by user");

    if(sidetone < 1 || sidetone > 9) {
        LOG("Invalid argument provided");
        return -2;
    }

    char buf[3];

    buf[0] = 0x02;
    buf[1] = _SIDETONE;
    buf[2] = sidetone;

    if(send_data(buf, 3, 3) < 0) return -1;

    _communication->sidetone = sidetone;

    LOG("Sidetone change succeded");

    return 0; 
}

/* Sets new channel spacing
  set spacing to either _SPACING833 or _SPACING25
  returns 0 on success, -1 on fault, -2 if incorrect arguments are provided */
int set_spacing(char spacing) {
    LOG("Channel spacing change requested by user");
    if(spacing != _SPACING25 && spacing != _SPACING833) {
        PRINT_ERROR_MSG("Invalid arguments provided");
        return -2;
    }

    char buf[2];

    buf[0] = 0x02;
    buf[1] = spacing;

    CHECKnp(send_data(buf, 2, 0));
    
    _communication->spacing=spacing;

    LOG("Channel spacing succeded");

    return 0;
}

/* Returns channel from khz interger */
unsigned char get_channel(int khz) {
    if(khz > 995) return 198;
    if(khz < 0) return 0;
    if (khz % 25 == 20) {
        khz = khz - 5;
    }
    int channel = khz / 5;
    return channel;
}

/* Returns khz integer from channel*/
int get_khz(unsigned char channel) {
    if(channel < 0) return 0;
    if(channel > 198) return 990;
    return channel*5;
}

/* Returns either "8.33" or "25" based on provided spacing*/
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
    #ifndef TEST
    char response = 0;
    int attempts = 0;
    
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