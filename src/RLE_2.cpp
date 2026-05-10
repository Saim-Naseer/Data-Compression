#include <iostream>
using namespace std;

void rle2_encode(unsigned char *input, size_t len, unsigned char *output, size_t *out_len)
{
  size_t j = 0;
  size_t i = 0;

  while (i < len)
  {
    if (input[i] == 0)
    {
      size_t count = 1;
      while (i + count < len && input[i + count] == 0 && count < 255)
        count++;

      output[j++] = 0;
      output[j++] = (unsigned char)count;
      i += count;

      while (i < len && input[i] == 0)
      {
        count = 1;
        while (i + count < len && input[i + count] == 0 && count < 255)
          count++;

        output[j++] = 0;
        output[j++] = (unsigned char)count;
        i += count;
      }
    }
    else
    {
      output[j++] = input[i++];
    }
  }

  *out_len = j;
}
void rle2_decode(unsigned char *input, size_t len, unsigned char *output, size_t *out_len)
{
  size_t j = 0;

  for (size_t i = 0; i < len; i++)
  {
    if (input[i] != 0)
    {
      output[j++] = input[i];
    }
    else
    {
      if (i + 1 >= len)
        break;
      unsigned char count = input[i + 1];
      for (size_t k = 0; k < count; k++)
        output[j++] = 0;
      i++;
    }
  }

  *out_len = j;
}
// int main()
// {
//   size_t len = 7;
//   unsigned char *input = new unsigned char[len];
//   input[0] = 0;
//   input[1] = 2;
//   input[2] = 0;
//   input[3] = 2;
//   input[4] = 3;
//   input[5] = 3;
//   input[6] = 0;
//   unsigned char *output = new unsigned char[2 * len];
//   size_t out_len;
//   rle2_encode(input, len, output, &out_len);
//   for (int i = 0; i < out_len; i++)
//   {
//     cout << (int)output[i] << " ";
//   }
//   cout << endl
//        << endl;
//   unsigned char *output2 = new unsigned char[2 * out_len];
//   size_t out_len2;
//   rle2_decode(output, out_len, output2, &out_len2);
//   for (int i = 0; i < out_len2; i++)
//   {
//     cout << (int)output2[i] << " ";
//   }

//   return 0;
// }