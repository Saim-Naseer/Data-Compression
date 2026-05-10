#include <iostream>
using namespace std;

void mtf_encode(unsigned char *input, size_t len, unsigned char *output)
{
  unsigned char arr[256];

  for (int i = 0; i < 256; i++)
  {
    arr[i] = i;
  }

  for (int i = 0; i < (int)len; i++)
  {
    for (int k = 0; k < 256; k++)
    {
      if ((int)input[i] == (int)arr[k])
      {
        output[i] = k;
        int temp = arr[k];
        for (int l = k; l > 0; l--)
        {
          arr[l] = arr[l - 1];
        }
        arr[0] = temp;
        break;
      }
    }
  }
}

void mtf_decode(unsigned char *input, size_t len, unsigned char *output)
{
  unsigned char arr[256];

  for (int i = 0; i < 256; i++)
  {
    arr[i] = i;
  }

  for (int i = 0; i < (int)len; i++)
  {
    output[i] = arr[input[i]];
    int temp = arr[input[i]];
    for (int l = input[i]; l > 0; l--)
    {
      arr[l] = arr[l - 1];
    }
    arr[0] = temp;
  }
}

// int main()
// {
//   size_t len = 3;
//   unsigned char *input = new unsigned char[len];
//   string s = "SAI";
//   for (int i = 0; i < len; i++)
//   {
//     input[i] = s[i];
//   }
//   unsigned char *output = new unsigned char[len];
//   mtf_encode(input, len, output);

//   cout << endl
//        << "Encoding";

//   cout << endl
//        << endl;

//   for (int i = 0; i < len; i++)
//   {
//     cout << endl
//          << (int)output[i];
//   }

//   cout << endl
//        << "Decoding";
//   unsigned char *output2 = new unsigned char[len];
//   mtf_decode(output, len, output2);

//   cout << endl
//        << endl;

//   for (int i = 0; i < len; i++)
//   {
//     cout << endl
//          << output2[i];
//   }
//   return 0;
// }