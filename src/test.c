#include "krt2-ctrl.h"
#include <string.h>

struct KRT2_communication comm;
struct KRT2_frequency freq;

int main(){
    krt_init("/dev/ttyS0", &freq, &comm);
    printf("KRT2 initialized\n");
    fflush(stdout);
    printf("\033[?25l");
    while(1){
        krt_check();
        printf("active: %i.%i %s\n", freq.active_frequency, get_khz(freq.active_channel), freq.active_name);
        fflush(stdout);
        printf("stby: %i.%i %s\n", freq.stby_frequency, get_khz(freq.stby_channel), freq.stby_name);
        fflush(stdout);
        printf("radio vol: %i, radio sql: %i, intercom vol: %i, intercom sql: %i\n", comm.volume, comm.squelch, comm.intercom_volume, comm.intercom_squelch);
        fflush(stdout);
        printf("external audio: %i, sidetone: %i, PTT: %i, spacing: %s kHz\n", comm.external_input, comm.sidetone, comm.PTT, get_spacing_str(comm.spacing));
        printf("\e[4A%c", 0xD);
        fflush(stdout);
        sleep(0.1);
    }
    return 0;
}