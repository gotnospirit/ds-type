#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdint.h>

int read_inputs();

int keypressed(uint32_t);
int keyheld(uint32_t);

float stick_dx();
float stick_dy();

#endif