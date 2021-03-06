#include <cassert>
#include <iostream>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

static_assert(TABLES_DEPTH > 1, "Impossible memory layout - table depth 1 is only sufficient to contain the root table");

#define RET_SUCCESS     1
#define RET_FAIL        0


uint64_t currentPath(uint64_t virtualAddress){
    return (virtualAddress >> OFFSET_WIDTH);
}
uint64_t currentOffset(uint64_t virtualAddress, uint64_t currentDepth){
    return ((virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - ((currentDepth + 1) * OFFSET_WIDTH)))
            & PAGE_BITMASK);
}

searchResult findUnusedFrame(word_t targetIndex) {
    minCyclicInfo cyclicInfo = {ROOT_TABLE_INDEX, ROOT_TABLE_INDEX, targetIndex, ROOT_TABLE_INDEX};
    innerSearchResult result = findUnusedFrame(0, ROOT_TABLE_INDEX, ROOT_TABLE_INDEX, cyclicInfo, ROOT_TABLE_INDEX);
    if (result.isEmpty && result.frameIndex > ROOT_TABLE_INDEX){
        // Empty frame is unused
        return searchResult{result.frameIndex, result.parentIndex};
    } else {
        // Next frame (maxUsed + 1) should be unused
        if (result.frameIndex + 1 < NUM_FRAMES){
            return searchResult{result.frameIndex + 1, TEMP_PARENT_INDEX};
        }
        // Must evict!
        else{
            // Evict frame and clear
            PMevict(cyclicInfo.frameIndex, cyclicInfo.pageIndex);
            // Return evicted frame
            return searchResult{cyclicInfo.frameIndex, cyclicInfo.parentIndex};
        }
    }
}
innerSearchResult findUnusedFrame(uint64_t depth, word_t frameIndex, word_t parentIndex,
                             minCyclicInfo &io_cyclicInfo, uint64_t pathToCurrent){
    innerSearchResult result{};
    result.frameIndex = frameIndex;
    result.parentIndex = parentIndex;

    // Leaf frame?
    if (depth == TABLES_DEPTH){
        // Update cyclic info if found better candidate for eviction
        if (io_cyclicInfo.frameIndex == ROOT_TABLE_INDEX ||
            cyclicDistance(frameIndex, io_cyclicInfo.targetIndex) < io_cyclicInfo.distance()){
            io_cyclicInfo.frameIndex = frameIndex;
            io_cyclicInfo.parentIndex = parentIndex;
            io_cyclicInfo.pageIndex = pathToCurrent;
        }
        result.isEmpty = false; // isFrameEmpty(frameIndex);
        return result;
    }

    // This is a table frame
    // Assume this table is empty, unless otherwise proved
    result.isEmpty = true;

    // Go over all rows in the table, recursively follow links until empty frame found
    word_t nextFrameIndex;
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {

        // Read next row in the table
        PMread(frameIndex * PAGE_SIZE + i, &nextFrameIndex);

        // Not empty?
        if (nextFrameIndex != ROOT_TABLE_INDEX){

            // This was table was not empty after all
            result.isEmpty = false;

            // This table was allocated for a parent? (which called this function to find a
            // frame for its child)
            if (nextFrameIndex == TEMP_PARENT_INDEX){
                //todo: update frameindex to max?
                continue;
            }

            // It links to another frame (must be non-negative index)
            assert(nextFrameIndex > 0);

            // Build path to next frame
            uint64_t pathToNext = ((pathToCurrent << OFFSET_WIDTH) | (i));

            // Search for unused frame through this link
            innerSearchResult nextResult = findUnusedFrame(depth + 1, nextFrameIndex, frameIndex,
                                                      io_cyclicInfo, pathToNext);

            // Found empty frame through link?
            if (nextResult.isEmpty){
                return nextResult;
            }

            // No empties found through link?
            // Remember maximal frame index visited and continue
            else{
                result.frameIndex = max(result.frameIndex, nextResult.frameIndex);
            }
        }
    }

    // Either this table was empty or not, result.frameIndex contains either the index of this
    // table or the maximal frame visited, accordingly.
    return result;
}

bool isFrameEmpty(uint64_t frameIndex){
    word_t value;

    // Go over all rows in the frame, checked if used or not
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {

        // Read next row in the frame
        PMread(frameIndex * PAGE_SIZE + i, &value);

        // Not empty?
        if (value != ROOT_TABLE_INDEX){
            return false;
        }
    }
    // All rows are empty!
    return true;
}

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

void VMinitialize() {
    clearTable(0);
}

word_t findLeaf(uint64_t virtualAddress) {
    word_t frameIndex = ROOT_TABLE_INDEX;
    uint64_t offset{0};
    word_t nextIndex{0};

    // Find and generate page tables on the way
    for (uint64_t depth = 0; depth < TABLES_DEPTH; depth++) {
        offset = currentOffset(virtualAddress, depth);
        PMread(frameIndex * PAGE_SIZE + offset, &nextIndex);
        if (nextIndex == 0) {
            PMwrite(frameIndex * PAGE_SIZE + offset, TEMP_PARENT_INDEX);
            searchResult result = findUnusedFrame(virtualAddress); // TODO: Handle evict if
            clearTable(result.frameIndex);

            nextIndex = result.frameIndex;

            // Unlink unused frame from parent if exists
            if (result.parentIndex != TEMP_PARENT_INDEX) {
                // Find the offsetOfChildInParent of the child within the parent table
                word_t childIndex;
                word_t offsetOfChildInParent{-1};
                for (word_t i = 0; i < PAGE_SIZE; ++i) {
                    PMread(result.parentIndex * PAGE_SIZE + i, &childIndex);
                    if (childIndex == result.frameIndex) {
                        offsetOfChildInParent = i;
                        break;
                    }
                }
                PMwrite(result.parentIndex * PAGE_SIZE + offsetOfChildInParent, 0);
            }

            // Unused frame is now the next page table
            // Write the index of the next page table in the current page table
            PMwrite(frameIndex * PAGE_SIZE + offset, nextIndex);
        }
        frameIndex = nextIndex;
    }
    PMrestore(frameIndex, (virtualAddress >> OFFSET_WIDTH));
    offset = currentOffset(virtualAddress, TABLES_DEPTH);

    // We now have a leaf!
    return (frameIndex * PAGE_SIZE + offset);
}

int VMread(uint64_t virtualAddress, word_t* value) {
    auto leaf = findLeaf(virtualAddress);
    PMread(leaf, value);
    return RET_SUCCESS;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    auto leaf = findLeaf(virtualAddress);
    PMwrite(leaf, value);
    return RET_SUCCESS;
}
