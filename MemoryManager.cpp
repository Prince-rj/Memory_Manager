#include "MemoryManager.h"
#include <iomanip> 
#include <sstream>
#include <iostream>
#include <algorithm>
using namespace std;

MemoryManager::MemoryManager(unsigned long long memorySize, unsigned int pageSize)
    : memorySize(memorySize), pageSize(pageSize) {
    numFrames = memorySize / pageSize;
    memory.resize(numFrames, 0); 
}

vector<string> MemoryManager::allocatePages(unsigned int numPages, bool isPageTable) {
    vector<string> allocatedPages;
    unsigned int allocated = 0;

    // Try to allocate pages
    for (unsigned int i = 0; i < numFrames && allocated < numPages; ++i) {
        if (memory[i] == 0) { // Check if the page is free
            if (isPageTable) memory[i] = 2;//if pagetable mark as 2
            else memory[i] = 1; // Mark as 1
            ostringstream addrStream;
            addrStream << "0x" << setw(8) << setfill('0') << hex << (i * pageSize);
            string addr = addrStream.str();
            allocatedPages.push_back(addr); // Store physical address as hex string
            ++allocated;
        }
    }

    // If not enough pages are allocated, try to free non-page table pages
    if (allocated < numPages) {
        //here we don't remove the page table from the memory but just the pages that are not of page table
        // also remove the page table and assign 
        for (unsigned int i = 0; i < numFrames && allocated < numPages; ++i) {
            if (memory[i] == 1) { // Check if the page is a non-page table page
                memory[i] = 0; // Free the page
                if (isPageTable) memory[i] = 2;
                else memory[i] = 1; // Mark as allocated
                ostringstream addrStream;
                addrStream << "0x" << setw(8) << setfill('0') << hex << (i * pageSize);
                string addr = addrStream.str();
                allocatedPages.push_back(addr); // Store physical address as hex string in the vector
                ++allocated;
            }
        }
    }

    // If still not enough pages are allocated, clear the list to indicate failure
    if (allocated < numPages) {
        allocatedPages.clear();
    }

    return allocatedPages;
}



unsigned int MemoryManager::getFreePageCount() const {
    // for(auto i:memory)cout<<i<<" ";
    return std::count(memory.begin(), memory.end(), 0);
}
