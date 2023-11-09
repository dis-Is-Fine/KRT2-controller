#include <stdio.h>
#include "serial.h"

struct KRT2_frequency {
    char active_frequency;
    char active_channel;
    char active_name[8];
    char stby_frequency;
    char stby_channel;
    char stby_name[8];
};

struct KRT2_communication {
    char volume;
    char squelch;
    char intercom_volume;
    char intercom_squelch;
    char external_input;
    char sidetone;
    char PTT;
};

int krt_init(char* file, struct KRT2_frequency* frequency,
            struct KRT2_communication* communication);