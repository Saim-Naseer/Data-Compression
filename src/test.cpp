#include "../include/Block.h"
#include "../include/BWT.h"
#include "../include/RLE_1.h"
#include "../include/MTF.h"
#include "../include/RLE_2.h"
#include "../include/Huffman.h"
#include "../include/inipp.h"

#include <fstream>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <ctime>

using namespace std;

/*
  Assumes:
  - BWT uses '$' as sentinel and input does not contain '$'
  - Huffman functions are updated to packed-byte output
*/

struct BlockMeta
{
  size_t after_rle1 = 0;
  size_t after_bwt = 0;
  size_t after_mtf = 0;
  size_t after_rle2 = 0;
  int bwt_index = -1;
  unsigned char huff_lengths[256]{};
};

static void replace_block_data(Block &block, unsigned char *new_data, size_t new_size)
{
  delete[] block.data;
  block.data = new_data;
  block.size = new_size;
}

static void free_huffman_tree(Node *root)
{
  if (!root)
    return;
  free_huffman_tree(root->left);
  free_huffman_tree(root->right);
  delete root;
}

static void print_block(const char *label, const unsigned char *data, size_t len)
{
  cout << label << endl;
  data = data;
  len = len;
  // cout << label << " : ";
  // for (size_t i = 0; i < len; i++)
  // {
  //   if (data[i] == '$')
  //   {
  //     cout << "$ ";
  //   }
  //   else if (isprint(static_cast<unsigned char>(data[i])))
  //   {
  //     cout << data[i] << " ";
  //   }
  //   else
  //   {
  //     cout << (int)data[i] << " ";
  //   }
  // }
  // cout << endl;
}

static size_t count_huffman_bits(const unsigned char lengths[256], const unsigned char *data, size_t len)
{
  size_t freq[256] = {0};

  for (size_t i = 0; i < len; i++)
    freq[data[i]]++;

  size_t bits = 0;
  for (int i = 0; i < 256; i++)
    bits += freq[i] * lengths[i];

  return bits;
}

static void write_block_record(FILE *bin, const Block &block, const BlockMeta &meta, const Config &cfg)
{
  unsigned char flags = 0;
  if (cfg.rle1_enabled)
    flags |= 1 << 0;
  if (cfg.bwt_enabled)
    flags |= 1 << 1;
  if (cfg.mtf_enabled)
    flags |= 1 << 2;
  if (cfg.rle2_enabled)
    flags |= 1 << 3;
  if (cfg.huffman_enabled)
    flags |= 1 << 4;

  fwrite(&flags, sizeof(flags), 1, bin);
  fwrite(&block.original_size, sizeof(size_t), 1, bin);
  fwrite(&meta.after_rle1, sizeof(size_t), 1, bin);
  fwrite(&meta.after_bwt, sizeof(size_t), 1, bin);
  fwrite(&meta.after_mtf, sizeof(size_t), 1, bin);
  fwrite(&meta.after_rle2, sizeof(size_t), 1, bin);
  fwrite(&meta.bwt_index, sizeof(int), 1, bin);

  if (cfg.huffman_enabled)
  {
    fwrite(meta.huff_lengths, sizeof(unsigned char), 256, bin);
  }

  fwrite(&block.size, sizeof(size_t), 1, bin);
  fwrite(block.data, sizeof(unsigned char), block.size, bin);
}

static bool read_block_record(FILE *bin, Block &block, BlockMeta &meta, bool &rle1, bool &bwt, bool &mtf, bool &rle2, bool &huffman)
{
  unsigned char flags = 0;
  if (fread(&flags, sizeof(flags), 1, bin) != 1)
    return false;

  rle1 = (flags >> 0) & 1;
  bwt = (flags >> 1) & 1;
  mtf = (flags >> 2) & 1;
  rle2 = (flags >> 3) & 1;
  huffman = (flags >> 4) & 1;

  fread(&block.original_size, sizeof(size_t), 1, bin);
  fread(&meta.after_rle1, sizeof(size_t), 1, bin);
  fread(&meta.after_bwt, sizeof(size_t), 1, bin);
  fread(&meta.after_mtf, sizeof(size_t), 1, bin);
  fread(&meta.after_rle2, sizeof(size_t), 1, bin);
  fread(&meta.bwt_index, sizeof(int), 1, bin);

  if (huffman)
    fread(meta.huff_lengths, sizeof(unsigned char), 256, bin);

  fread(&block.size, sizeof(size_t), 1, bin);
  delete[] block.data;
  block.data = new unsigned char[block.size];
  fread(block.data, sizeof(unsigned char), block.size, bin);
  return true;
}

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    return 1;
  }
  bool compress_mode = (strcmp(argv[1], "-c") == 0);
  char input_path[512], output_path[512];
  if (compress_mode)
    snprintf(input_path,  sizeof(input_path),  "benchmarks/%s", argv[2]);
  else
    snprintf(input_path,  sizeof(input_path),  "results/%s", argv[2]);
  snprintf(output_path, sizeof(output_path), "results/%s", argv[3]);

  Config cfg = read_config();

  if (compress_mode)
  {
    FILE *clear = fopen(output_path, "wb");
    if (clear)
      fclose(clear);

    BlockManager *BM = divide_into_blocks(input_path, cfg.block_size);
    if (!BM)
    {
      cerr << "Failed to load input blocks.\n";
      return 1;
    }

    // double start = clock();
    BlockMeta *meta = new BlockMeta[BM->num_blocks]{};

    cout << endl
         << "Num Blocks : " << BM->num_blocks << endl;

    cout << endl
         << "=================" << endl
         << "Encoding" << endl
         << "=================" << endl;

    FILE *bin = fopen(output_path, "ab");
    if (!bin)
    {
      cerr << "Could not open results/compressed.bin for writing.\n";
      free_block_manager(BM);
      delete[] meta;
      return 1;
    }

    for (int i = 0; i < BM->num_blocks; i++)
    {
      cout << endl
           << "---------- BLOCK " << i << " ----------" << endl;

      Block &blk = BM->blocks[i];
      blk.original_size = blk.size;

      print_block("Init", blk.data, blk.size);

      // RLE1
      if (cfg.rle1_enabled)
      {
        unsigned char *out = new unsigned char[2 * blk.size];
        size_t out_len = 0;
        rle1_encode(blk.data, blk.size, out, &out_len);
        replace_block_data(blk, out, out_len);
      }
      meta[i].after_rle1 = blk.size;
      print_block("RLE1", blk.data, blk.size);

      // BWT
      if (cfg.bwt_enabled)
      {
        unsigned char *out = new unsigned char[blk.size + 1];
        int index = -1;
        bwt_encode(blk.data, blk.size, out, &index);
        meta[i].bwt_index = index;
        replace_block_data(blk, out, blk.size + 1);
      }
      meta[i].after_bwt = blk.size;
      print_block("BWT", blk.data, blk.size);

      // MTF
      if (cfg.mtf_enabled)
      {
        unsigned char *out = new unsigned char[blk.size];
        mtf_encode(blk.data, blk.size, out);
        replace_block_data(blk, out, blk.size);
      }
      meta[i].after_mtf = blk.size;
      print_block("MTF", blk.data, blk.size);

      // RLE2
      if (cfg.rle2_enabled)
      {
        unsigned char *out = new unsigned char[2 * blk.size];
        size_t out_len = 0;
        rle2_encode(blk.data, blk.size, out, &out_len);
        replace_block_data(blk, out, out_len);
      }
      meta[i].after_rle2 = blk.size;
      print_block("RLE2", blk.data, blk.size);

      // Huffman
      if (cfg.huffman_enabled)
      {
        Node *root = build_huffman_tree(blk.data, blk.size);

        unsigned char lengths[256] = {0};
        HuffmanCode codes[256] = {};

        get_code_lengths(root, 0, lengths);
        generate_canonical_codes(lengths, codes);

        memcpy(meta[i].huff_lengths, lengths, 256);

        size_t bits = count_huffman_bits(lengths, blk.data, blk.size);
        size_t packed_bytes = (bits + 7) / 8;
        if (packed_bytes == 0)
          packed_bytes = 1;

        unsigned char *huff_out = new unsigned char[packed_bytes];
        size_t huff_len = 0;
        huffman_encode(blk.data, blk.size, codes, huff_out, &huff_len);

        replace_block_data(blk, huff_out, huff_len);

        free_huffman_tree(root);
        print_block("Huffman", blk.data, blk.size);
      }

      write_block_record(bin, blk, meta[i], cfg);
    }

    fclose(bin);
  }

  if (!compress_mode)
  {
    cout << endl
         << "=================" << endl
         << "Decoding" << endl
         << "=================" << endl;

    FILE *dec_bin = fopen(input_path, "rb");
    FILE *dec_out = fopen(output_path, "wb");
    if (!dec_bin || !dec_out)
    {
      cerr << "Could not open files for decoding.\n";
      return 1;
    }

    BlockMeta bm2 = {};
    Block blk2 = {};
    bool u_rle1, u_bwt, u_mtf, u_rle2, u_huffman;
    int i = 0;

    while (read_block_record(dec_bin, blk2, bm2, u_rle1, u_bwt, u_mtf, u_rle2, u_huffman))
    {
      cout << endl
           << "---------- BLOCK " << i++ << " ----------" << endl;

      if (u_huffman)
      {
        HuffmanCode decode_codes[256] = {};
        generate_canonical_codes(bm2.huff_lengths, decode_codes);
        unsigned char *out = new unsigned char[bm2.after_rle2];
        size_t out_len = 0;
        huffman_decode(blk2.data, blk2.size, decode_codes, out, bm2.after_rle2, &out_len);
        replace_block_data(blk2, out, out_len);
      }

      if (u_rle2)
      {
        unsigned char *out = new unsigned char[bm2.after_mtf];
        size_t out_len = 0;
        rle2_decode(blk2.data, blk2.size, out, &out_len);
        replace_block_data(blk2, out, out_len);
      }

      if (u_mtf)
      {
        unsigned char *out = new unsigned char[bm2.after_bwt];
        mtf_decode(blk2.data, blk2.size, out);
        replace_block_data(blk2, out, blk2.size);
      }

      if (u_bwt)
      {
        unsigned char *out = new unsigned char[bm2.after_rle1];
        size_t out_len = 0;
        bwt_decode(blk2.data, blk2.size, bm2.bwt_index, out, &out_len);
        replace_block_data(blk2, out, out_len);
      }

      if (u_rle1)
      {
        unsigned char *out = new unsigned char[blk2.original_size];
        size_t out_len = 0;
        rle1_decode(blk2.data, blk2.size, out, &out_len);
        replace_block_data(blk2, out, out_len);
      }

      fwrite(blk2.data, 1, blk2.size, dec_out);
      delete[] blk2.data;
      blk2.data = nullptr;
    }

    fclose(dec_bin);
    fclose(dec_out);
  }

  if (compress_mode)
  {
    FILE *orig = fopen(input_path, "rb");
    fseek(orig, 0, SEEK_END);
    long original_size = ftell(orig);
    fclose(orig);

    FILE *comp = fopen(output_path, "rb");
    fseek(comp, 0, SEEK_END);
    long compressed_size = ftell(comp);
    fclose(comp);

    double ratio = (double)compressed_size / original_size;
    double percent = (1.0 - ratio) * 100.0;

    cout << endl;
    cout << "Original Size: " << original_size << endl;
    cout << "Compressed Size: " << compressed_size << endl;
    cout << "Compression Ratio: " << ratio << endl;
    cout << "Compression Percentage: " << percent << "%" << endl;
  }
  return 0;
}