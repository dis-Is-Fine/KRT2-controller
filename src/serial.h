#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

int serial_init(char* file, int baud);
int serial_write(char* msg, int size);
int serial_read(char* buf, int buf_size, int required_bytes);
int non_canonical_set(int min_bytes, int max_time);
int serial_readB(char* buf);
void serial_end();