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

int krt_init(char* file, struct KRT2_frequency* _frequency,
            struct KRT2_communication* _communication);

int krt_check();

unsigned char get_channel(int khz);
int get_khz(unsigned char channel);

char* get_spacing_str(char spacing);