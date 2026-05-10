#include <iostream>
using namespace std;

void rle2_encode(unsigned char *input, size_t len, unsigned char *output, size_t *out_len);
void rle2_decode(unsigned char *input, size_t len, unsigned char *output, size_t *out_len);