# Data Compression Pipeline Project

## Overview

This project implements a complete block-based lossless data compression pipeline in C++ using multiple classical compression stages:

1. Run-Length Encoding 1 (RLE-1)
2. Burrows-Wheeler Transform (BWT)
3. Move-To-Front Transform (MTF)
4. Run-Length Encoding 2 (RLE-2)
5. Canonical Huffman Coding

The system reads an input text file, divides it into configurable blocks, applies the compression pipeline sequentially, stores the compressed binary output, and then performs full decompression to reconstruct the original file.

The implementation was developed completely from scratch without using external compression libraries.

---

# Features

- Configurable block-based compression
- Binary-safe file handling
- Full compression and decompression pipeline
- Burrows-Wheeler Transform implementation
- Move-To-Front encoding/decoding
- Dual Run-Length Encoding stages
- Canonical Huffman Coding
- Compression statistics generation
- Configurable pipeline stages through `config.ini`
- Cross-platform support (Linux + Windows)
- Automatic reconstruction and verification of decompressed data

---

# Compression Pipeline

The project uses the following pipeline:

Input File  
→ Block Division  
→ RLE-1  
→ BWT  
→ MTF  
→ RLE-2  
→ Huffman Coding  
→ Compressed Binary File

During decompression, the reverse order is applied:

Compressed Binary File  
→ Huffman Decoding  
→ RLE-2 Decoding  
→ MTF Decoding  
→ BWT Decoding  
→ RLE-1 Decoding  
→ Reconstructed File

---

# Project Structure

```text
project/
│
├── include/
│   ├── Block.h
│   ├── BWT.h
│   ├── Huffman.h
│   ├── MTF.h
│   ├── RLE_1.h
│   ├── RLE_2.h
│   └── inipp.h
│
├── src/
│   ├── Block.cpp
│   ├── BWT.cpp
│   ├── Huffman.cpp
│   ├── MTF.cpp
│   ├── RLE_1.cpp
│   ├── RLE_2.cpp
│   └── test.cpp
│
├── benchmarks/
│   └── data.txt
│
├── results/
│   ├── compressed.bin
│   ├── enc.txt
│   └── dec.txt
│
├── config.ini
├── Makefile
└── README.md
```

---

# Detailed Implementation

## 1. Block Management

### Files

- `Block.h`
- `Block.cpp`

### Purpose

The input file is divided into fixed-size blocks before compression. This improves memory management and allows each block to be compressed independently.

### Implementation Details

The project defines two custom structures:

```cpp
typedef struct
{
    unsigned char *data;
    size_t size;
    size_t original_size;
} Block;
```

```cpp
typedef struct
{
    Block *blocks;
    int num_blocks;
    size_t block_size;
} BlockManager;
```

### Key Functions

#### `divide_into_blocks()`

- Reads the entire file in binary mode
- Splits data into configurable blocks
- Stores original block sizes for decompression

#### `reassemble_blocks()`

- Combines decompressed blocks
- Writes reconstructed output file

#### `free_block_manager()`

- Frees dynamically allocated memory safely

---

# 2. Run-Length Encoding Stage 1 (RLE-1)

### Files

- `RLE_1.h`
- `RLE_1.cpp`

### Purpose

The first RLE stage compresses consecutive repeating characters before BWT.

Example:

```text
AAAAABBBCC
```

Becomes:

```text
(A,5)(B,3)(C,2)
```

### Implementation Details

The encoder:

- Tracks the current character
- Counts repetitions
- Stores `(character, count)` pairs

### Key Functions

#### `rle1_encode()`

Compresses repeating sequences.

#### `rle1_decode()`

Reconstructs the original sequence.

### Notes About Implementation

- Counts are stored as unsigned bytes
- Output buffer size is allocated dynamically
- Original block size is preserved for safe decoding

---

# 3. Burrows-Wheeler Transform (BWT)

### Files

- `BWT.h`
- `BWT.cpp`

### Purpose

BWT rearranges characters to create long runs of similar values, improving later compression stages.

### Implementation Details

The implementation:

- Generates all cyclic rotations of the block
- Sorts rotations lexicographically using `qsort`
- Extracts the last column
- Stores the primary index required for decoding

### Encoding Steps

For a string:

```text
BANANA$
```

output is:

```text
annb$aa,4
```

The algorithm:

1. Generates all rotations
2. Sorts them
3. Stores the final column

### Decoding Details

The decoder:

- Rebuilds the first column by sorting
- Constructs LF-mapping
- Traverses rows using the primary index
- Reconstructs the original string

### Important Internal Details

- Sentinel character `$` is appended during encoding
- Primary index is stored alongside transformed data
- Dynamic memory allocation is used throughout the stage

---

# 4. Move-To-Front Transform (MTF)

### Files

- `MTF.h`
- `MTF.cpp`

### Purpose

MTF converts repeated symbols into small integers after BWT.

This produces many zeros and low-valued numbers, improving RLE-2 and Huffman compression.

### Implementation Details

The implementation maintains a list of all 256 byte values.

For each symbol:

1. Find its index
2. Output the index
3. Move the symbol to the front

### Example

Input:

```text
annb$aa,4
```

Output:

```text
0202330
```

Produces many low-valued integers after BWT.

### Key Functions

#### `mtf_encode()`

Encodes symbols into positions.

#### `mtf_decode()`

Reconstructs symbols using the same dynamic list logic.

---

# 5. Run-Length Encoding Stage 2 (RLE-2)

### Files

- `RLE_2.h`
- `RLE_2.cpp`

### Purpose

After MTF, many zeros appear. RLE-2 specifically compresses runs of zeros.

### Implementation Details

Encoding rule:

```text
0 0 0 0
```

Becomes:

```text
0 4
```

### Key Functions

#### `rle2_encode()`

Compresses zero runs.

#### `rle2_decode()`

Restores zeros correctly.

### Design Choice

This stage is intentionally specialized for zeros because MTF naturally produces many zero values after BWT.

---

# 6. Canonical Huffman Coding

### Files

- `Huffman.h`
- `Huffman.cpp`

### Purpose

Final entropy compression stage.

### Implementation Details

The implementation:

- Calculates symbol frequencies
- Builds a Huffman tree
- Generates code lengths
- Produces canonical Huffman codes
- Encodes data into packed bitstreams

### Structures Used

```cpp
typedef struct
{
    uint64_t code;
    unsigned char length;
} HuffmanCode;
```

### Key Algorithms

#### Huffman Tree Construction

Uses frequency-based merging of nodes.

#### Canonical Code Generation

Symbols are sorted by:

1. Code length
2. Symbol value

This guarantees deterministic code assignment.

### Binary Bit Packing

Unlike simple `'0'/'1'` string storage, the implementation:

- Packs bits into bytes
- Tracks valid bit count
- Reduces storage overhead significantly

### Stored Metadata

For each block:

- Original block size
- Huffman code lengths
- Encoded bitstream size
- Packed binary stream

---

# Configuration System

## File

`config.ini`

### Example

```ini
[ General ]
block_size = 1000
rle1_enabled = true
bwt_enabled = true
mtf_enabled = true
rle2_enabled = true
huffman_enabled = true
```

### Supported Settings

| Setting           | Description                    |
| ----------------- | ------------------------------ |
| `block_size`      | Size of each compression block |
| `rle1_enabled`    | Enable first RLE stage         |
| `bwt_enabled`     | Enable BWT                     |
| `mtf_enabled`     | Enable MTF                     |
| `rle2_enabled`    | Enable second RLE stage        |
| `huffman_enabled` | Enable Huffman coding          |

---

# Build Instructions

## Linux

### Requirements

- g++
- make

### Build

```bash
make
```

### Run

```bash
make run
```

### Clean

```bash
make clean
```

---

# Windows

## Using MinGW

### Build

```bash
mingw32-make
```

### Run

```bash
./app
```

### Clean

```bash
mingw32-make clean
```

---

# Cross Compilation for Windows

From Linux:

```bash
make windows
```

---

# Usage Example

## Input File

```text
Sample text file
Sample text file
Sample text file
```

## Execution

```bash
make run
```

## Generated Files

| File             | Description                         |
| ---------------- | ----------------------------------- |
| `compressed.bin` | Binary compressed output            |
| `enc.txt`        | Intermediate encoded representation |
| `dec.txt`        | Final decompressed output           |

---

# Compression Statistics

The program automatically prints:

```text
Original Size: XXXX bytes
Compressed Size: XXXX bytes
Compression Ratio: X.XX
Compression Percentage: XX%
```

---

# Experimental Results

## Dataset

The benchmark file contains repetitive textual data to test:

- RLE effectiveness
- BWT clustering
- MTF locality
- Huffman entropy reduction

---

# Observed Results

| Stage   | Effect                         |
| ------- | ------------------------------ |
| RLE-1   | Compresses repeated characters |
| BWT     | Groups similar symbols         |
| MTF     | Produces many zeros            |
| RLE-2   | Compresses zero runs           |
| Huffman | Final entropy reduction        |

---

# Compression Graphs

| 1 Mb File Compression Results    |
| -------------------------------- |
| Original Size: 1108364           |
| Compressed Size: 81275           |
| Compression Ratio: 0.0733288     |
| Compression Percentage: 92.6671% |
| Execution time: 82.841 sec       |

# Team Members and Contributions

## Talal Nadeem

- Phase 1

## Saim Naseer

- Phase 2

## Abdullah Yasir

- Phase 3

---

# Conclusion

This project demonstrates the implementation of a complete multi-stage lossless compression system using classical compression algorithms. The final pipeline successfully compresses repetitive text data while maintaining exact reconstruction during decompression.

The project also provided practical experience with:

- Dynamic memory management
- Binary file handling
- Entropy coding
- Transform-based compression
- Algorithm integration
- Debugging low-level systems code
