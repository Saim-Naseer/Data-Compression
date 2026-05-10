#include <iostream>
using namespace std;
/*
 * Structure to hold a single block of data
3 */
typedef struct
{
  unsigned char *data;  // Pointer to block data
  size_t size;          // Current size of block
  size_t original_size; // Original size before compression
} Block;

/*
 * Structure to manage multiple blocks
 */
typedef struct
{
  Block *blocks;     // Array of blocks
  int num_blocks;    // Number of blocks
  size_t block_size; // Configurable block size
} BlockManager;

typedef struct
{
  int block_size;
  bool rle1_enabled;
  bool bwt_enabled;
  bool mtf_enabled;
  bool rle2_enabled;
  bool huffman_enabled;

  bool benchmark_mode;
  bool output_metrics;

  std::string input_directory;
  std::string output_directory;
} Config;

Config read_config();
BlockManager *divide_into_blocks(const char *filename, size_t block_size);
int reassemble_blocks(BlockManager *manager, const char *output_filename);
void free_block_manager(BlockManager *manager);
