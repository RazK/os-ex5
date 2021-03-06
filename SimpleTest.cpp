#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <cstdio>
//#include <cassert>

void test_0(){

    PMwrite(0 * PAGE_SIZE + 0, 1);
    PMwrite(1 * PAGE_SIZE + 1, 2);
    PMwrite(2 * PAGE_SIZE + 1, 3);
    PMwrite(3 * PAGE_SIZE + 0, 4);

    PMwrite(1 * PAGE_SIZE + 0, 5);
    PMwrite(5 * PAGE_SIZE + 1, 6);
    PMwrite(6 * PAGE_SIZE + 1, 7);

    searchResult result;
    result = findUnusedFrame(31);
    printf("UNUSED FRAME : %d", result.frameIndex);
}

/**
    * VIRTUAL_ADDRESS_WIDTH 12
 */
void test_1(){
    uint64_t test = 0b010100010110;
    uint64_t result0 = currentOffset(test, 0);
    uint64_t result1 = currentOffset(test, 1);
    uint64_t result2 = currentOffset(test, 2);
    printf("PAGE_BITMASK : %d\r\n", PAGE_BITMASK);
    printf("RESULT0 : %lld\r\n", result0);
    printf("RESULT1 : %lld\r\n", result1);
    printf("RESULT2 : %lld\r\n", result2);
}

/**
 *  VIRTUAL_ADDRESS_WIDTH 5
 *  OFFSET_WIDTH 1
 *  PHYSICAL_ADDRESS_WIDTH 4
 */
void test_2(){
    VMwrite(13, 12345);
    printRAM();
    word_t val;
    VMread(13, &val);
    printf("READ VALUE : %d", val);
}

void test_3(){
    VMwrite(13, 12345);
    //printRAM();
    word_t val;
    VMread(13, &val);
    printf("READ VALUE : %d\n", val);
    PMwrite(14, 6789);
    VMread(6, &val);
    printf("READ VALUE : %d\n", val);
    VMread(31, &val);
//    printRAM();
    printf("READ VALUE : %d\n", val);
}

int test_simple()
{
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        if (uint64_t(value) != i){
            printf("oh no!");
            return 0;
        }
    }
    printf("success\n");

    return 0;
}

int test_simple2()
{
    for (uint64_t i = 0; i < (20 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (20 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        if (uint64_t(value) != i){
            printf("oh no!");
            return 0;
        }
    }
    printf("success\n");

    return 0;
}

int main(int argc, char **argv) {
    VMinitialize();
    test_simple2();
//    test_3();
}

/*
int main(int argc, char **argv) {
    VMinitialize();
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        assert(uint64_t(value) == i);
    }
    printf("success\n");

    return 0;
}
*/