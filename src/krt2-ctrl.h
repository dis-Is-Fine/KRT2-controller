#include <stdio.h>
#include "serial.h"

#define _ACT_FREQ 'U'
#define _STBY_FREQ 'R'
#define _COMM_CFG 'A'
#define _PTT '2'
#define _IC_VOL '3'
#define _EXT_VOL '4'
#define _SIDETONE '1'
#define _SPACING833 '8'
#define _SPACING25 '6'
#define _STATUS_BAT 'B'
#define _STATUS_BAT_CNCL 'D'
#define _STATUS_RX 'J'
#define _STATUS_RX_CNCL 'V'
#define _STATUS_TX 'K'
#define _STATUS_RX_TXCNCL 'Y'
#define _STATUS_TIMEOUT 'L'
#define _STATUS_DUAL_ON 'O'
#define _STATUS_DUAL_OFF 'o'
#define _STATUS_DUALRX_ACT 'M'
#define _STATUS_DUALRX_STBY 'm'
#define _ERROR_ADC 'a'
#define _ERROR_ANTENNA 'b'
#define _ERROR_FPAA 'c'
#define _ERROR_FREQ_SYNTH 'd'
#define _ERROR_PLL 'e'
#define _ERROR_INPUT_BLK 'f'
#define _ERROR_I2C_BUS 'g'
#define _ERROR_D10_DIODE 'h'
#define _ERROR_CLEAR 'F'

struct KRT2_frequency {
    char active_frequency;
    char active_channel;
    char active_name[9];
    char stby_frequency;
    char stby_channel;
    char stby_name[9];
};

struct KRT2_communication {
    char volume;
    char squelch;
    char intercom_volume;
    char intercom_squelch;
    char external_input;
    char sidetone;
    char PTT;
    char spacing;
};

#define _MASK_BAT 0b00000001
#define _MASK_RX 0b00000010
#define _MASK_TX 0b00000100
#define _MASK_TIMEOUT 0b00001000
#define _MASK_DUAL_ON 0b00010000
#define _MASK_DUAL_RX_ACT 0b00100000
#define _MASK_DUAL_RX_STBY 0b01000000
#define _MASK_DUAL_RX 0b01100000

#define _MASK_ERROR_ADC 0b00000001
#define _MASK_ERROR_ANTENNA 0b00000010
#define _MASK_ERROR_FPAA 0b00000100
#define _MASK_ERROR_FREQ_SYNTH 0b00001000
#define _MASK_ERROR_PLL 0b00010000
#define _MASK_ERROR_INPUT_BLK 0b00100000
#define _MASK_ERROR_IC2_BUS 0b01000000
#define _MASK_ERROR_D10_DIODE 0b10000000

int krt_init(char* file, struct KRT2_frequency* _frequency,
            struct KRT2_communication* _communication, char* status, char* error);

int krt_check();

unsigned char get_channel(int khz);
int get_khz(unsigned char channel);

char* get_spacing_str(char spacing);

int set_active_frequency(char frequency, char channel, char name[9]);