#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool is_pow_2(uint32_t i);
uint32_t next_pow_2(uint32_t i);
float linear_ease_in(int, int);
int clamp(int, int, int);
char * read_file(const char *);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif