#include "../include/BWT.h"

#include <cstring>
#include <cstdlib>

static int compare_rotations(const void *a, const void *b)
{
  char *r1 = *(char **)a;
  char *r2 = *(char **)b;

  return strcmp(r1, r2);
}

static int compare_chars(const void *a, const void *b)
{
  return (*(char *)a - *(char *)b);
}

void bwt_encode(const unsigned char *input, size_t len, unsigned char *output, int *primary_index)
{
  unsigned char *temp = new unsigned char[len + 1];

  for (size_t i = 0; i < len; i++)
  {
    temp[i] = input[i];
  }

  temp[len] = '$';

  char **rotations = new char *[len + 1];

  for (size_t i = 0; i < len + 1; i++)
  {
    rotations[i] = new char[len + 2];

    for (size_t j = 0; j < len + 1; j++)
    {
      rotations[i][j] = temp[j];
    }

    rotations[i][len + 1] = '\0';

    unsigned char first = temp[0];

    for (size_t j = 0; j < len; j++)
    {
      temp[j] = temp[j + 1];
    }

    temp[len] = first;
  }

  qsort(rotations, len + 1, sizeof(char *), compare_rotations);

  for (size_t i = 0; i < len + 1; i++)
  {
    output[i] = (unsigned char)rotations[i][len];

    if (rotations[i][len] == '$')
    {
      *primary_index = (int)i;
    }
  }

  for (size_t i = 0; i < len + 1; i++)
  {
    delete[] rotations[i];
  }

  delete[] rotations;
  delete[] temp;
}

void bwt_decode(const unsigned char *input, size_t len, int primary_index, unsigned char *output, size_t *out_len)
{
  char *L = new char[len];
  char *F = new char[len];

  for (size_t i = 0; i < len; i++)
  {
    L[i] = input[i];
    F[i] = input[i];
  }

  qsort(F, len, sizeof(char), compare_chars);

  int *next = new int[len];

  bool used[10000] = {false};

  for (size_t i = 0; i < len; i++)
  {
    for (size_t j = 0; j < len; j++)
    {
      if (!used[j] && L[i] == F[j])
      {
        next[i] = (int)j;
        used[j] = true;
        break;
      }
    }
  }

  int current = primary_index;

  unsigned char *reversed = new unsigned char[len];

  for (size_t i = 0; i < len; i++)
  {
    reversed[i] = L[current];
    current = next[current];
  }

  size_t j = 0;

  for (int i = (int)len - 1; i >= 0; i--)
  {
    if (reversed[i] != '$')
    {
      output[j++] = reversed[i];
    }
  }

  *out_len = j;

  delete[] reversed;
  delete[] next;
  delete[] L;
  delete[] F;
}