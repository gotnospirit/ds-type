#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdint.h>

int read_inputs();

int pressed(uint32_t);
int held(uint32_t);
int released(uint32_t);

float stick_dx();
float stick_dy();

#endif