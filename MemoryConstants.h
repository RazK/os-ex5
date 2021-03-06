#pragma once

#include <climits>
#include <stdint.h>

// word
typedef int word_t;

#define WORD_WIDTH (sizeof(word_t) * CHAR_BIT)

// number of bits in the offset,
//#define OFFSET_WIDTH 1
//#define OFFSET_WIDTH 4
#define OFFSET_WIDTH 8

// page/frame size in words
// in this implementation this is also the number of entries in a table
#define PAGE_SIZE (1LL << OFFSET_WIDTH)


// number of bits in a physical address
//#define PHYSICAL_ADDRESS_WIDTH 4
//#define PHYSICAL_ADDRESS_WIDTH 10
#define PHYSICAL_ADDRESS_WIDTH 20


// RAM size in words
#define RAM_SIZE (1LL << PHYSICAL_ADDRESS_WIDTH)

// number of bits in a virtual address
//#define VIRTUAL_ADDRESS_WIDTH 5
//#define VIRTUAL_ADDRESS_WIDTH 20
#define VIRTUAL_ADDRESS_WIDTH 40


// virtual memory size in words
#define VIRTUAL_MEMORY_SIZE (1LL << VIRTUAL_ADDRESS_WIDTH)

// number of frames in the RAM
#define NUM_FRAMES (RAM_SIZE / PAGE_SIZE)

// number of pages in the virtual memory
#define NUM_PAGES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)

#define TABLES_DEPTH ((VIRTUAL_ADDRESS_WIDTH - 1) / OFFSET_WIDTH)
