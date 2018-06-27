#pragma once

#include "MemoryConstants.h"

#define ROOT_TABLE_INDEX 0

typedef enum _searchConclusion{
    UNINITIALIZED,
    MUST_EVICT,
    FOUND_EMPTY
} searchConclusion;

/*
 * Usage state of a frame
 */
typedef enum _frameUsage{
    TABLE,      // Frame is used as a page table
    USED_FRAME,	// Frame has value or points at another frame (i.e. frame is a table)
    EMPTY, 	    // Frame = 0
    ON_HOLD     // We allocated it on the path to another frame and it is still in a fragile, uninitialized state
} frameUsage;

/*
 * Usage state
 * If page is used, also contains cyclic distance from page we were trying to access
 */
typedef struct _frameStatus{
    // is the frame empty? on hold? used?
    frameUsage usage;

    // cyclic distance from page we're trying to access:
    // min{NUM_PAGES - |page_swapped_in - p|, |page_swapped_in - p|}
    uint64_t cyclicDistance;
} frameStatus;

typedef struct _frameSearchResult{
    // All frames are used, must evict?
    searchConclusion conclusion;

    // Maximal used frame
    uint64_t maxUsedFrameIndex;

    // Index corresponding to frame number
    frameStatus resultFrames[TABLES_DEPTH];
} frameSearchResult;

typedef struct _searchResult{
    bool isEmpty;
    uint64_t frameIndex; // isEmpty ? emtpyIndex : maxVisitedIndex
} searchResult;

/*
 * min{NUM_PAGES - |page_swapped_in - p|, |page_swapped_in - p|}
 */
inline uint64_t cyclicDistance(uint64_t page_swapped_in, uint64_t p);

/*
 * Return searchResult of traversing the page hierarchy for empty frames
 */

/*
 * Return searchResult of traversing the page hierarchy for empty frames
 */
searchResult findUnusedFrame();
searchResult findUnusedFrame(uint64_t depth, uint64_t frameIndex);

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


