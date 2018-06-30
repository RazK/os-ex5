#pragma once

#include "MemoryConstants.h"

#define ROOT_TABLE_INDEX 0
#define TEMP_PARENT_INDEX (-1)
#define PAGE_BITMASK ((1 << OFFSET_WIDTH) - 1)

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define abs(x) ((x) < 0 ? (-1*(x)) : (x))

void clearTable(uint64_t frameIndex);

inline word_t cyclicDistance(word_t page_swapped_in, word_t p) {
    return min(NUM_PAGES - abs(page_swapped_in - p), abs(page_swapped_in - p));
}

typedef struct _minCyclicInfo{
    word_t frameIndex;
    word_t parentIndex;
    word_t targetIndex;
    uint64_t pageIndex;
    inline word_t distance(){ return cyclicDistance(frameIndex, targetIndex);};
} minCyclicInfo;

typedef struct _searchResult{
    word_t frameIndex;
    word_t parentIndex;
} searchResult;

typedef struct _innerSearchResult{
    bool isEmpty;
    word_t frameIndex;        // isEmpty ? emtpyIndex : maxVisitedIndex
    word_t parentIndex;
} innerSearchResult;

/**
 *
 * @param virtualAddress
 * @param currentDepth
 * @return
 */
uint64_t currentOffset(uint64_t virtualAddress, uint64_t currentDepth);

/*
 * min{NUM_PAGES - |page_swapped_in - p|, |page_swapped_in - p|}
 */
inline uint64_t cyclicDistance(uint64_t page_swapped_in, uint64_t p);

/*
 * Return innerSearchResult of traversing the page hierarchy for empty frames
 */
searchResult findUnusedFrame(word_t targetIndex);
innerSearchResult findUnusedFrame(uint64_t depth, word_t frameIndex, word_t parentIndex, minCyclicInfo &io_cyclicInfo, uint64_t pathToCurrent);

/*
 * Are all the words in this frame = zero?
 */
bool isFrameEmpty(uint64_t frameIndex);

/*
 * Initialize the virtual memory
 */
void VMinitialize();

/* reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value);

/* writes a word to the given virtual address
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */

int VMwrite(uint64_t virtualAddress, word_t value);

/**
 * Logic for finding an empty table (leaf) to use
 * @param virtualAddress
 * @return
 */
word_t findLeaf(uint64_t virtualAddress);


