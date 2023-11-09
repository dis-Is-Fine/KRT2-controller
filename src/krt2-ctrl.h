#include <stdio.h>
#include "serial.h"

#define SPACING833 0
#define SPACING25 1

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
    char spacing;
};

int krt_init(char* file, struct KRT2_frequency* frequency,
            struct KRT2_communication* communication);