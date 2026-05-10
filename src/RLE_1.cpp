#include <iostream>
using namespace std;

void rle1_encode(unsigned char *input, size_t len, unsigned char *output, size_t *out_len)
{
  size_t j = 0;
  size_t i = 0;

  while (i < len)
  {
    unsigned char c = input[i];
    size_t count = 1;

    while (i + count < len && input[i + count] == c && count < 255)
      count++;

    output[j++] = c;
    output[j++] = (unsigned char)count;
    i += count;

    while (i < len && input[i] == c)
    {
      count = 1;
      while (i + count < len && input[i + count] == c && count < 255)
        count++;

      output[j++] = c;
      output[j++] = (unsigned char)count;
      i += count;
    }
  }

  *out_len = j;
}
void rle1_decode(unsigned char *input, size_t len, unsigned char *output, size_t *out_len)
{
  size_t j = 0;
  if (len % 2 != 0)
  {
    *out_len = 0;
    return;
  }

  for (size_t i = 0; i < len; i += 2)
  {
    unsigned char c = input[i];
    size_t count = input[i + 1];
    for (size_t k = 0; k < count; k++)
      output[j++] = c;
  }

  *out_len = j;
}
// int main()
// {
//   size_t len = 9;
//   unsigned char *input = new unsigned char[len];
//   string s = "ABBBCCCCD";
//   for (int i = 0; i < len; i++)
//   {
//     input[i] = s[i];
//   }
//   unsigned char *output = new unsigned char[len];
//   size_t *out_len;
//   rle1_encode(input, len, output, out_len);
//   cout << *out_len << endl;
//   for (int i = 0; i < *out_len; i++)
//   {
//     if (i % 2 == 1)
//     {
//       cout << (int)output[i];
//     }
//     else
//     {
//       cout << output[i];
//     }
//   }
//   cout << endl;

//   unsigned char *output_2 = new unsigned char[len];
//   size_t *out_len_2;
//   rle1_decode(output, *out_len, output_2, out_len_2);
//   cout << *out_len_2 << endl;
//   for (int i = 0; i < *out_len_2; i++)
//   {
//     cout << output_2[i];
//   }
//   cout << endl;

//   return 0;
// }