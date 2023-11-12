#include "krt2-ctrl.h"
#include <string.h>

int main(){
    struct KRT2_communication comm;
    struct KRT2_frequency freq;
    krt_init("/dev/ttyS2", &freq, &comm);
    freq.active_channel = 20;
    freq.active_frequency = 119;
    char buf[9] = "XXXX_ABC";
    for(int i = 0; i < 9; i++){
        freq.active_name[i] = buf[i];
    }
    printf("KRT2 initialized\n");
    while(1){
        krt_check();
        printf("active: %i.%i %s%c", freq.active_frequency, get_khz(freq.active_channel), freq.active_name, 0xD);
        fflush(stdout);
        sleep(0.1);
    }
}