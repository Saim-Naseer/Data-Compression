#ifndef BWT_H
#define BWT_H

#include <stddef.h>

void bwt_encode(const unsigned char *input,
                size_t len,
                unsigned char *output,
                int *primary_index);

void bwt_decode(const unsigned char *input,
                size_t len,
                int primary_index,
                unsigned char *output,
                size_t *out_len);

#endif