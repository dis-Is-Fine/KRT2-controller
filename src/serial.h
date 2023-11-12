#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

int serial_init(char* file, int baud);
int serial_write(char* msg, int size);
int serial_read(char* buf, int buf_size);
int non_canonical_set(int min_bytes, int max_time);
int serial_readB(char* buf);
void serial_end();

#define CHECK(x) ({int __val = (x); if (__val < 0) { \
    fprintf(stderr, "Runtime error at %s:%d\nError %d (%s)\nCaused by: %s\n", __FILE__, __LINE__, errno, strerror(errno), #x); \
    return __val;}})

#define CHECKa(x, y) ({int __val = (x); if (__val < 0) { \
    fprintf(stderr, "Runtime error at %s:%d\nError %d (%s)\nCaused by: %s\n", __FILE__, __LINE__, errno, strerror(errno), #x); \
    return __val;} *y = __val;})
