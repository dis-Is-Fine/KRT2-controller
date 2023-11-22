#include "serial.h"

int serial_port;
struct termios tty;

/* Initializes serial port from 'file' file
 and with 'baud' baudrate (baudrates are defined in termios.h)
 returns 0 on success and -1 on fault */
int serial_init(char* file, int baud) {

    char log_buf[50];
    snprintf(log_buf, 50, "Initializing serial port %s", file);
    LOG(log_buf);

    CHECKa(open(file, O_RDWR | O_NOCTTY), &serial_port);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_lflag &= ~(ECHO | ECHOE);
    tty.c_lflag &= ~ICANON;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag &= ~CRTSCTS;
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VTIME] = 30;
    tty.c_cc[VMIN] = 0;

    CHECK(cfsetspeed(&tty, baud));

    CHECK(tcsetattr(serial_port, TCSANOW, &tty));

    LOG("Serial port initialization succeded");

    return 0;
}

/* Sets how to handle non-canonical reads
 'min_bytes describes c_cc VMIN
 'max_time describes c_cc VTIME
  refer to termios manual for more info 
  returns 0 on success and -1 on fault */
int non_canonical_set(int min_bytes, int max_time) {

    tty.c_cc[VTIME] = max_time;
    tty.c_cc[VMIN] = min_bytes;
    
    CHECK(tcsetattr(serial_port, TCSANOW, &tty));

    return 0;
}

/* Writes 'size' bytes from 'msg' to serial port
 returns number of bytes written 
 in case of fault returns -1 */
int serial_write(char* msg, int size) {

    int n_bytes;
    CHECKa(write(serial_port, msg, size), &n_bytes);

    if(n_bytes != size){LOG("Sent less data than required");}

    return n_bytes;

}

/* Reads 'buf size' bytes into 'buf'
  returns number of bytes read
  before use non canonical reads should be configured
  using non_canonical_set() - refer to termios manual
  on fault returns -1 */
int serial_read(char* buf, int buf_size) {

    int n_bytes;
    CHECKa(read(serial_port, buf, buf_size), &n_bytes);

    return n_bytes;

}

/* Reads byte of data into 'buf'
  returns number of bytes read
  on fault returns -1 */
int serial_readB(char* buf) {
    return serial_read(buf, 1);
}

/* Closes serial port */
void serial_end() {

    close(serial_port);
    LOG("Serial port closed");

}

/* Logs message to log file
  put your message in 'message'
  put line number and file of code
  in respective arguments
  best to use from LOG() macro */
int logger(char* message, char* file, int line) {
    int fd = 0;
    CHECKa(open("test.log", O_CREAT | O_WRONLY | O_APPEND, 0777), &fd);
    int len = strlen(message)+strlen(file)+10;
    char buf[len];
    sprintf(buf, "%s:%d : %s\n", file, line, message);
    CHECK(write(fd, buf, strlen(buf)));
    CHECK(close(fd));
    return 0;
}

/* Clears contents of log file */
int clear_log() {
    CHECK(truncate("./test.log", 0));
    return 0;
}