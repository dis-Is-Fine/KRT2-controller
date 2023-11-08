#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

void serial_init(char* file);
int serial_write(char* msg, int size);
int serial_read(char* buf, int buf_size);