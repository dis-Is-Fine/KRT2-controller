# KRT2-controller
This is a simple linux library for communicating with KRT2 radios by TQ-Group
To use it, include files in 'src' folder in your project

## Initializing the library
To initialize library first allocate memory for following data:
- `struct KRT2_frequency`
- `struct KRT2_communication`
- `char status`
- `char error`

after that call `krt_init()` and set:
- `char* file` to serial port file
- rest of the arguments as pointers to previously allocated data
  
**Remember to check for faults!**

Example:
```C
struct KRT2_frequency frequency;
struct KRT2_communication communication;
char status;
char error;

int ret_val = krt_init("/dev/ttyS0", &frequency, &communication, &status, &error);
if (ret_val < 0) {
  printf("KRT2 initialization failed");
}
```



## Using the library

while using the library you should call `krt_check()` to check for incoming transmissions from KRT2 radio
`krt_check()` shouldn't be called less frequently than 50 ms
when calling `krt_check()` check for any faults (returns -1)

to send any commands to KRT2 radio use following functions:
- `set_frequency()` to set active or standby frequency (and frequency name)
- `set_new_communication_cfg()` to set volume, squelch or intercom (VOX) squelch
- `set_PTT()` to set PTT button
- `set_intercom_vol()` to set intercom (VOX) volume
- `set_ext_audio_vol()` to set external audio input volume
- `set_sidetone()` to set sidetone
- `set_spacing()` to set channel spacing (8.33 kHz or 25 kHz)

for more detail check function comments

to get data about current radio status read `KRT2_frequency` and `KRT2_communication` structs
or `status` and `error` chars (To decypher them use _MASKs defined in `krt2-ctrl.h`)

## Example useage

To see example program check `test.c` in `/test` folder

## Milestones:

- [x] Handle incoming data from KRT2 radio
- [x] Allow to send commands to KRT2 radio
- [x] Create example program
- [ ] Remove need to call `krt_check` manually
- [ ] Add support for receive only commands in KRT2 radio
