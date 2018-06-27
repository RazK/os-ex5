#include "PhysicalMemory.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cstdio>

typedef std::vector<word_t> page_t;

std::vector<page_t> RAM;
std::unordered_map<uint64_t, page_t> swapFile;

void setupTest(){
    // TODO: Remove after done testing
    PMwrite(0 * PAGE_SIZE + 0, 1);
    PMwrite(1 * PAGE_SIZE + 1, 2);
    PMwrite(2 * PAGE_SIZE + 1, 3);
    PMwrite(3 * PAGE_SIZE + 0, 4);

    PMwrite(1 * PAGE_SIZE + 0, 5);
    PMwrite(5 * PAGE_SIZE + 1, 6);
    PMwrite(6 * PAGE_SIZE + 1, 7);
}

void initialize() {
    RAM.resize(NUM_FRAMES, page_t(PAGE_SIZE));
    setupTest();
}

void PMread(uint64_t physicalAddress, word_t* value) {
    if (RAM.empty())
        initialize();

    assert(physicalAddress < RAM_SIZE);

    *value = RAM[physicalAddress / PAGE_SIZE][physicalAddress
             % PAGE_SIZE];
 }

void PMwrite(uint64_t physicalAddress, word_t value) {
    if (RAM.empty())
        initialize();

    assert(physicalAddress < RAM_SIZE);

    RAM[physicalAddress / PAGE_SIZE][physicalAddress
             % PAGE_SIZE] = value;
}

void PMevict(uint64_t frameIndex, uint64_t evictedPageIndex) {

    if (RAM.empty())
        initialize();

    assert(swapFile.find(evictedPageIndex) == swapFile.end());
    assert(frameIndex < NUM_FRAMES);
    assert(evictedPageIndex < NUM_PAGES);

    swapFile[evictedPageIndex] = RAM[frameIndex];
}

void PMrestore(uint64_t frameIndex, uint64_t restoredPageIndex) {
    if (RAM.empty())
        initialize();

    assert(frameIndex < NUM_FRAMES);

    // page is not in swap file, so this is essentially
    // the first reference to this page. we can just return
    // as it doesn't matter if the page contains garbage
    if (swapFile.find(restoredPageIndex) == swapFile.end())
        return;

    RAM[frameIndex] = std::move(swapFile[restoredPageIndex]);
    swapFile.erase(restoredPageIndex);
}
