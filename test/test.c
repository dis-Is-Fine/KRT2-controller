#define DEBUG

#include "../src/krt2-ctrl.h"
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

struct KRT2_communication comm;
struct KRT2_frequency freq;
struct termios term_default;
struct termios temp;
char status;
char error;
char last_pressed;
long last_checksum = -1;
long checksum = 0;

void exitf(); int set_instant_stdin(char instant);
void print_status(); void print_choice();
int main_loop(); int radio_set();
int set_radio_frequency(char selector);
int set_volume(); int set_other();

int main(){
    clear_log();
    /* Setup fuctions to execute at exit */
    signal(SIGINT, exitf);
    signal(SIGQUIT, exitf);
    atexit(exitf);
    /* Save current stdin attributes */
    CHECK(tcgetattr(STDIN_FILENO, &term_default));
    /* Initialize KRT2 radio */
    LOG("Initializing KRT2");
    if(krt_init("/dev/ttyS0", &freq, &comm, &status, &error) < 0) return -1;
    LOG("KRT2 initialized successfully");
    #ifdef DEBUG
        printf("%sKRT2 initialized\n%s", _COLOR_GREEN_T, _COLOR_DEFAULT_T);
        sleep(1);
    #endif
    printf("\033[?25l"); /* Disable cursor */
    /* Execute main loop */
    CHECKnp(main_loop());
    return 0;
}

void data_checksum() {
    checksum = 0;
    for(int i = 0; i < sizeof(comm); i++){
        checksum += ((char*) &comm)[i]; 
    }
    for(int i = 0; i < sizeof(freq); i++){
        checksum += ((char*) &freq)[i]; 
    }
    checksum += error + status;
}

/* At exit restore original stdin attributes and show cursor */
void exitf() {
    fflush(stdout);
    #ifdef DEBUG
        sleep(3);
    #endif
    printf("%s%s", _SHOW_CURSOR, _CLEAR_SCREEN);
    set_instant_stdin(0);
    serial_end();
}

int main_loop(){
    while(1){
        krt_check();
        set_instant_stdin(1);
        read(STDIN_FILENO, &last_pressed, 1);
        set_instant_stdin(0);
        if((last_pressed >= 'a' && last_pressed <= 'e') ||
            (last_pressed >= 'A' && last_pressed <= 'E')) {
                CHECKnp(radio_set());
                last_checksum = -1;
                LOG("returned");
                last_pressed = 0;
        }
        data_checksum();
        if(checksum != last_checksum) {
            last_checksum = checksum;
            print_status();
            printf("\e[6A%c", 0xD);
        }
    }
}

int radio_set() {
    printf("%s", _CLEAR_SCREEN);
    print_choice();
    switch(last_pressed) {
        case 'A': CHECKnp(set_radio_frequency(0)); break;
        case 'a': CHECKnp(set_radio_frequency(0)); break;
        case 'B': CHECKnp(set_radio_frequency(1)); break;
        case 'b': CHECKnp(set_radio_frequency(1)); break;
        case 'C': CHECKnp(set_volume()); break;
        case 'c': CHECKnp(set_volume()); break;
        case 'D': CHECKnp(set_other()); break;
        case 'd': CHECKnp(set_other()); break;
        case 'E': exit(0);
        case 'e': exit(0);
        default: return 0;
    }
    return 0;
}

/* If instant == 0 restore original stdin attributes
  else set stdin to return avaiable data instantly*/
int set_instant_stdin(char instant) {
    if(instant == 0){
        CHECK(tcsetattr(STDIN_FILENO, TCSANOW,&term_default));
        return 0;
    }
    memcpy(&temp, &term_default, sizeof(term_default));
    temp.c_lflag &= ~(ICANON | ECHO); 
    temp.c_cc[VMIN] = 0;
    temp.c_cc[VTIME] = 0;
    CHECK(tcsetattr(STDIN_FILENO, TCSANOW, &temp));
    return 0;
}

/* Print radio status and choice menu */
void print_status(){
    printf("%s", _CLEAR_SCREEN);
    printf("active: %i.%i %s\n", freq.active_frequency, get_khz(freq.active_channel), freq.active_name);
    printf("stby: %i.%i %s\n", freq.stby_frequency, get_khz(freq.stby_channel), freq.stby_name);
    printf("radio vol: %i, radio sql: %i, intercom vol: %i, intercom sql: %i    \n", comm.volume, comm.squelch, comm.intercom_volume, comm.intercom_squelch);
    printf("external audio: %i, sidetone: %i, PTT: %i, spacing: %s kHz    \n", comm.external_input, comm.sidetone, comm.PTT, get_spacing_str(comm.spacing));
    printf("errors: %8b, status: %8b    \n", error, status);
    print_choice();
    fflush(stdout);
}

/* Print action menu */
void print_choice() {
    switch(last_pressed){
        case 'A': printf("%sA - Active freq%s | B - Stby freq | C - Volume | D - Other | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        case 'a': printf("%sA - Active freq%s | B - Stby freq | C - Volume | D - Other | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        case 'B': printf("A - Active freq | %sB - Stby freq%s | C - Volume | D - Other | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        case 'b': printf("A - Active freq | %sB - Stby freq%s | C - Volume | D - Other | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        case 'C': printf("A - Active freq | B - Stby freq | %sC - Volume%s | D - Other | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        case 'c': printf("A - Active freq | B - Stby freq | %sC - Volume%s | D - Other | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        case 'D': printf("A - Active freq | B - Stby freq | C - Volume | %sD - Other%s | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        case 'd': printf("A - Active freq | B - Stby freq | C - Volume | %sD - Other%s | E - Exit", _COLOR_GREEN_T, _COLOR_DEFAULT_T); break;
        default: printf("A - Active freq | B - Stby freq | C - Volume | D - Other | E - Exit"); break;
    }
}

int set_radio_frequency(char selector){
    char buf[10] = {0};
    unsigned char freq = 0; unsigned char channel = 0;

    printf("\nEnter desired frequency (XXX.XXX): ");
    scanf("%7s", buf); getchar();
    if(buf[3] != '.' && buf[3] != ',') {
        return -1;
    }
    buf[3] = 0;
    freq = atoi(buf);
    channel = get_channel(atoi(buf+4));

    memset(buf, 0, 10);
    printf("Enter frequency name: ");
    scanf("%8[^\n]s", buf);
    CHECKnp(set_frequency(freq, channel, buf, selector));
    CHECK(set_frequency(freq, channel, buf, selector));
    return 0;
}

void get_input(int* buf, char* msg){
    printf(msg);
    int input = 0;
    scanf("%d", &input); getchar();
    while(input > 100) {input /= 10;}
    *buf = input;
    return;
}

int set_volume(){
    printf("\n");
    int volume; int squelch; int intercom_volume;
    int intercom_squelch; int external_input; int sidetone;
    get_input(&volume, "Enter volume: ");
    get_input(&squelch, "Enter squelch: ");
    get_input(&intercom_volume, "Enter intercom volume: ");
    get_input(&intercom_squelch, "Enter intercom squelch: ");
    get_input(&sidetone, "Enter external input: ");
    get_input(&external_input, "Enter sidetone: ");
    
    CHECKnp(set_new_communication_cfg(volume, squelch, intercom_squelch));
    CHECKnp(set_intercom_vol(intercom_volume));
    CHECKnp(set_ext_audio_vol(external_input));
    CHECKnp(set_sidetone(sidetone));

    return 0;
}

int set_other(){
    int PTT; int spacing;

    printf("\nEnter PTT (0 - pilot | 1 - copilot | 2 - both): ");
    scanf("%d", &PTT); getchar();

    printf("\nEnter channel spacing (8 or 25): ");
    scanf("%d", &spacing); getchar();

    if(PTT >= 0 && PTT <= 2) CHECKnp(set_PTT((char) PTT));
    switch(spacing){
        case 25: LOG("25"); CHECKnp(set_spacing(_SPACING25)); break;
        case 8: LOG("8"); CHECKnp(set_spacing(_SPACING833)); break;
        default: LOG("DEFAULT"); return 0;
    }
    return 0;
}