#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  uint64_t code;
  unsigned char length;
} HuffmanCode;

typedef struct Node
{
  int value;
  int freq;
  struct Node *left;
  struct Node *right;
} Node;

Node *build_huffman_tree(unsigned char *data, size_t len);

void get_code_lengths(Node *root,
                      int depth,
                      unsigned char lengths[256]);

void generate_canonical_codes(unsigned char lengths[256],
                              HuffmanCode codes[256]);

void huffman_encode(const unsigned char *input,
                    size_t len,
                    HuffmanCode codes[256],
                    unsigned char *output,
                    size_t *out_len);

void huffman_decode(const unsigned char *input,
                    size_t len,
                    HuffmanCode codes[256],
                    unsigned char *output,
                    size_t expected_out_len,
                    size_t *out_len);

#endif