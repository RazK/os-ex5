#include <cassert>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define abs(x) ((x) < 0 ? (-1*(x)) : (x))


static_assert(TABLES_DEPTH > 1, "Impossible memory layout - table depth 1 is only sufficient \
to contain the root table");


//frameSearchResult findEmptyFrame() {
//
//}
//
//void recursiveFindEmptyFrame(uint64_t destIndex, uint64_t curIndex, uint64_t layer,
//                             frameSearchResult& out_result){
//    assert(layer <= TABLES_DEPTH);
//
//    // Status of current frame
//    frameStatus myStatus{};
//
//    // Calc cyclic distance from dest
//    myStatus.cyclicDistance = cyclicDistance(curIndex, destIndex);
//
//    // Reached leaf frame?
//    if (layer == TABLES_DEPTH){
//
//        // Not empty?
//        if (!isFrameEmpty(curIndex)){
//            myStatus.usage = frameUsage::USED_FRAME;
//            out_result.resultFrames[layer] = myStatus;
//
//            // As far as this frame can see, we must evict something!
//            out_result.conclusion = MUST_EVICT;
//            return;
//
//        }
//        // Empty frame found!
//        else {
//            myStatus.usage = frameUsage::EMPTY;
//            out_result.resultFrames[layer] = myStatus;
//
//            // Found empty frame!
//            out_result.conclusion = FOUND_EMPTY;
//            return;
//        }
//    }
//
//    // In table on path to leaf
//    else {
//
//        // Not empty?
//        if (!isFrameEmpty(curIndex)){
//            myStatus.usage = frameUsage::TABLE;
//            out_result.resultFrames[layer] = myStatus;
//
//            // Recursively go into frames pointed by table
//            for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
//
//                // Calc index of i'th table
//                uint64_t nextIndex = curIndex * PAGE_SIZE + i;
//
//                // Not empty?
//                if (value != ROOT_TABLE_INDEX){
//                    return false;
//                }
//            }
//            return;
//        }
//        // Empty table found!
//        else {
//            myStatus.usage = frameUsage::EMPTY;
//            out_result.resultFrames[layer] = myStatus;
//
//            // Found empty frame!
//            out_result.conclusion = FOUND_EMPTY;
//            return;
//        }
//    }
//}

uint64_t currentOffset(uint64_t virtualAddress, uint64_t currentDepth){
    return (virtualAddress >> VIRTUAL_ADDRESS_WIDTH - (currentDepth + 1) * OFFSET_WIDTH) &
           PAGE_BITMASK;
}

word_t findUnusedFrame() {
    searchResult result = findUnusedFrame(0, ROOT_TABLE_INDEX);
    if (result.isEmpty && result.frameIndex > ROOT_TABLE_INDEX){
        // Empty frame is unused
        return result.frameIndex;
    } else {
        // Next frame should be unused
        return result.frameIndex + 1;
    }
}
searchResult findUnusedFrame(uint64_t depth, word_t frameIndex){
    searchResult result{};
    result.frameIndex = frameIndex;

    // Leaf frame?
    if (depth == TABLES_DEPTH){
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
                continue;
            }

            // It links to another frame (must be non-negative index)
            assert(nextFrameIndex > 0);

            // Search for unused frame through this link
            searchResult nextResult = findUnusedFrame(depth + 1, nextFrameIndex);

            // Found empty frame through link?
            if (nextResult.isEmpty){
                return nextResult;
            }

            // No empties found through link?
            // Remember maximal frame index visited and continue
            else{
                result.frameIndex = max(result.frameIndex, nextResult.frameIndex);;
            }
        }
    }

    // Either this table was empty or not, result.frameIndex contains either the index of this
    // table or the maximal frame visited, accordingly.
    return result;
}

inline uint64_t cyclicDistance(uint64_t page_swapped_in, uint64_t p){
    return min(NUM_PAGES - abs(page_swapped_in - p), abs(page_swapped_in - p));
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

word_t findLeaf(uint64_t virtualAddress){
    word_t frameIndex = ROOT_TABLE_INDEX;
    uint64_t offset;
    word_t nextIndex;

    // Find and generate page tables on the way
    for (uint64_t depth = 0; depth < TABLES_DEPTH; depth ++){
        offset = currentOffset(virtualAddress, depth);
        PMread(frameIndex * PAGE_SIZE + offset, &nextIndex);
        if (nextIndex == 0){
            PMwrite(frameIndex * PAGE_SIZE + offset, TEMP_PARENT_INDEX);
            nextIndex = findUnusedFrame(); // TODO: Handle evict if frameIndex > maxIndex
            clearTable(nextIndex);
            PMwrite(frameIndex * PAGE_SIZE + offset, nextIndex);
            frameIndex = nextIndex;
        }
    }

    // We now have a leaf!
    return (frameIndex * PAGE_SIZE + offset);
}

int VMread(uint64_t virtualAddress, word_t* value) {
    PMread(findLeaf(virtualAddress), value);
    return 0;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    PMwrite(findLeaf(virtualAddress), value);
    return 1;
}
