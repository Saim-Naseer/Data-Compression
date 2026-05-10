#include "../include/Huffman.h"

#include <cstring>

Node *create_node(int value, int freq)
{
  Node *node = new Node;
  node->value = value;
  node->freq = freq;
  node->left = nullptr;
  node->right = nullptr;
  return node;
}

Node *build_huffman_tree(unsigned char *data, size_t len)
{
  int freq[256] = {0};

  for (size_t i = 0; i < len; i++)
  {
    freq[data[i]]++;
  }

  Node *nodes[256];
  int count = 0;

  for (int i = 0; i < 256; i++)
  {
    if (freq[i] > 0)
    {
      nodes[count++] = create_node(i, freq[i]);
    }
  }

  if (count == 0)
  {
    return nullptr;
  }

  if (count == 1)
  {
    Node *root = create_node(-1, nodes[0]->freq);
    root->left = nodes[0];
    return root;
  }

  while (count > 1)
  {
    int min1 = 0;
    int min2 = 1;

    if (nodes[min2]->freq < nodes[min1]->freq)
    {
      int t = min1;
      min1 = min2;
      min2 = t;
    }

    for (int i = 2; i < count; i++)
    {
      if (nodes[i]->freq < nodes[min1]->freq)
      {
        min2 = min1;
        min1 = i;
      }
      else if (nodes[i]->freq < nodes[min2]->freq)
      {
        min2 = i;
      }
    }

    Node *left = nodes[min1];
    Node *right = nodes[min2];

    Node *merged = create_node(-1, left->freq + right->freq);
    merged->left = left;
    merged->right = right;

    nodes[min1] = merged;
    nodes[min2] = nodes[count - 1];
    count--;
  }

  return nodes[0];
}

void get_code_lengths(Node *root, int depth, unsigned char lengths[256])
{
  if (!root)
    return;

  if (!root->left && !root->right)
  {
    lengths[root->value] = (depth == 0) ? 1 : depth;
    return;
  }

  get_code_lengths(root->left, depth + 1, lengths);
  get_code_lengths(root->right, depth + 1, lengths);
}

void generate_canonical_codes(unsigned char lengths[256],
                              HuffmanCode codes[256])
{
  for (int i = 0; i < 256; i++)
  {
    codes[i].code = 0;
    codes[i].length = 0;
  }

  int symbols[256];
  int count = 0;

  for (int i = 0; i < 256; i++)
  {
    if (lengths[i] > 0)
    {
      symbols[count++] = i;
    }
  }

  if (count == 0)
    return;

  for (int i = 0; i < count - 1; i++)
  {
    for (int j = i + 1; j < count; j++)
    {
      if (lengths[symbols[i]] > lengths[symbols[j]] ||
          (lengths[symbols[i]] == lengths[symbols[j]] &&
           symbols[i] > symbols[j]))
      {
        int t = symbols[i];
        symbols[i] = symbols[j];
        symbols[j] = t;
      }
    }
  }

  uint64_t code = 0;
  int prev_len = lengths[symbols[0]];

  codes[symbols[0]].code = 0;
  codes[symbols[0]].length = prev_len;

  for (int i = 1; i < count; i++)
  {
    int sym = symbols[i];
    int len = lengths[sym];

    code++;

    code <<= (len - prev_len);

    codes[sym].code = code;
    codes[sym].length = len;

    prev_len = len;
  }
}

void huffman_encode(const unsigned char *input, size_t len, HuffmanCode codes[256], unsigned char *output, size_t *out_len)
{
  size_t j = 0;

  unsigned char current_byte = 0;
  int bit_count = 0;

  for (size_t i = 0; i < len; i++)
  {
    unsigned char sym = input[i];

    for (int b = codes[sym].length - 1; b >= 0; b--)
    {
      unsigned char bit =
          (unsigned char)((codes[sym].code >> b) & 1ULL);

      current_byte = (current_byte << 1) | bit;

      bit_count++;

      if (bit_count == 8)
      {
        output[j++] = current_byte;
        current_byte = 0;
        bit_count = 0;
      }
    }
  }

  if (bit_count > 0)
  {
    current_byte <<= (8 - bit_count);
    output[j++] = current_byte;
  }

  *out_len = j;
}

void huffman_decode(const unsigned char *input, size_t len, HuffmanCode codes[256], unsigned char *output, size_t expected_out_len, size_t *out_len)
{
  size_t j = 0;

  uint64_t current_code = 0;
  int current_bits = 0;

  for (size_t i = 0; i < len && j < expected_out_len; i++)
  {
    unsigned char byte = input[i];

    for (int b = 7; b >= 0 && j < expected_out_len; b--)
    {
      unsigned char bit = (byte >> b) & 1U;

      current_code = (current_code << 1) | bit;
      current_bits++;

      for (int s = 0; s < 256; s++)
      {
        if (codes[s].length == current_bits &&
            codes[s].code == current_code)
        {
          output[j++] = (unsigned char)s;

          current_code = 0;
          current_bits = 0;

          break;
        }
      }
    }
  }

  *out_len = j;
}