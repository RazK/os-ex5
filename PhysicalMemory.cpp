#include "PhysicalMemory.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cstdio>
#include <iostream>

typedef std::vector<word_t> page_t;

std::vector<page_t> RAM;
std::unordered_map<uint64_t, page_t> swapFile;


void initialize() {
    RAM.resize(NUM_FRAMES, page_t(PAGE_SIZE));
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

    // see if this evicted page is still in the swapFile
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
    if (swapFile.find(restoredPageIndex) == swapFile.end()){
        return;

    }

    RAM[frameIndex] = std::move(swapFile[restoredPageIndex]);
    swapFile.erase(restoredPageIndex);
}


void printRAM(){
    printf("RAM SIZE : %lld\r\n", RAM_SIZE);
    printf("==============\r\n");
    for (int i = 0; i < NUM_FRAMES; i++){
        printf("--------\r\n");
        printf("PAGE %d\r\n", i);
        printf("--------\r\n");
        for (int j = 0; j < PAGE_SIZE; j++){
            printf("RAM[%d][%d]\t= %d\r\n", i, j, RAM[i][j]);
        }
    }
}

void printSWAP(){
    printf("SWAP SIZE : %ld\r\n", swapFile.size());
    printf("==============\r\n");
    for (int j = 0; j < swapFile.size(); j++){
        auto i = 0;
        for (auto val:swapFile[j]){
            printf ("swapFile[%d][%d] = %d\n", j, i, val);
            i++;
        }
    }

//    for (int i = 0; i < NUM_FRAMES; i++){
//        printf("--------\r\n");
//        printf("PAGE %d\r\n", i);
//        printf("--------\r\n");
//        for (int j = 0; j < PAGE_SIZE; j++){
//            printf("RAM[%d][%d]\t= %d\r\n", i, j, RAM[i][j]);
//        }
//    }
}
