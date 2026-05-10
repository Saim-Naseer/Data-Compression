#include <iostream>
#include <fstream>
#include "../include/inipp.h"
#include "../include/Block.h"
#include <sstream>
#include <string>
using namespace std;

Config read_config()
{
  inipp::Ini<char> ini;
  ifstream is("config.ini");
  ini.parse(is);
  ini.strip_trailing_comments();

  Config cfg{};

  auto &gen = ini.sections[" General "];
  auto &perf = ini.sections[" Performance "];
  auto &paths = ini.sections[" Paths "];

  inipp::get_value(gen, "block_size", cfg.block_size);
  inipp::get_value(gen, "rle1_enabled", cfg.rle1_enabled);
  inipp::get_value(gen, "bwt_enabled", cfg.bwt_enabled);
  inipp::get_value(gen, "mtf_enabled", cfg.mtf_enabled);
  inipp::get_value(gen, "rle2_enabled", cfg.rle2_enabled);
  inipp::get_value(gen, "huffman_enabled", cfg.huffman_enabled);

  inipp::get_value(perf, "benchmark_mode", cfg.benchmark_mode);
  inipp::get_value(perf, "output_metrics", cfg.output_metrics);

  inipp::get_value(paths, "input_directory", cfg.input_directory);
  inipp::get_value(paths, "output_directory", cfg.output_directory);

  return cfg;
}

string read_file(const char *filename)
{
  ifstream file(filename, ios::in | ios::binary);
  if (file.is_open())
  {
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }
  cerr << "Could not open file: " << filename << endl;
  return "";
}

BlockManager *divide_into_blocks(const char *filename, size_t block_size)
{
  string data = read_file(filename);
  int byteCount = data.size();
  int num_blocks = (byteCount + block_size - 1) / block_size;
  BlockManager *manager = new BlockManager;
  manager->blocks = new Block[num_blocks];
  manager->num_blocks = num_blocks;

  for (int i = 0; i < num_blocks; i++)
  {
    Block block;
    if (byteCount >= (int)block_size)
    {
      block.size = block_size;
      block.original_size = block_size;
      byteCount = byteCount - block_size;
    }
    else
    {
      block.size = byteCount;
      block.original_size = byteCount;
      byteCount = 0;
    }
    block.data = new unsigned char[block.size];
    for (int j = 0; j < (int)block.size; j++)
    {
      block.data[j] = data[(i * block_size) + j];
    }

    manager->blocks[i] = block;
  }

  return manager;
}

int reassemble_blocks(BlockManager *manager, const char *output_filename)
{
  ofstream file(output_filename, ios::out | ios::binary);
  if (!file.is_open())
    return -1;

  for (int i = 0; i < manager->num_blocks; i++)
  {
    file.write(reinterpret_cast<char *>(manager->blocks[i].data),
               manager->blocks[i].size);
  }
  return 0;
}

void free_block_manager(BlockManager *manager)
{
  for (int i = 0; i < manager->num_blocks; i++)
  {
    delete[] manager->blocks[i].data;
  }
  delete[] manager->blocks;
  delete manager;
}

// int main()
// {
//   BlockManager *BM = divide_into_blocks("src/test.txt", 3);
//   reassemble_blocks(BM, "src/text2.txt");
//   // for (int i = 0; i < BM->num_blocks; i++)
//   // {
//   //   for (int j = 0; j < BM->blocks[i].size; j++)
//   //   {
//   //     cout << BM->blocks[i].data[j];
//   //   }
//   //   cout << endl;
//   // }
//   return 0;
// }

// int main()
// {
//   ifstream myFile;
//   myFile.open("test.txt", ios::out);
//   if (myFile.is_open())
//   {
//   }
//   Config cfg = read_config();
//   cout << "Block Size: " << cfg.block_size << endl;
//   return 0;
// }