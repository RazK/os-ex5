cmake_minimum_required(VERSION 3.8)
project(os_ex5)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES SimpleTest.cpp PhysicalMemory.cpp PhysicalMemory.h MemoryConstants.h VirtualMemory.cpp
        VirtualMemory.h)
add_executable(os_ex5 ${SOURCE_FILES})