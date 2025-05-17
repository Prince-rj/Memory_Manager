#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <limits>
#include <string>
#include <sstream>
#include <iomanip>
#include "MemoryManager.h"
#include "Performance.h"

using namespace std;

//string addressToHex(unsigned int address);
string addressToHex(unsigned int address, unsigned long long logicalMemorySize);

struct Frame {
    string physicalAddress;
};

class Task {
public:
    Task(std::string TaskID, unsigned long long logicalMemorySize, MemoryManager& memoryManager, unsigned int pageSize, int pageType, Performance &performance);
    ~Task(); // Destructor to free memory allocated for two-level page table
    vector<string> allocateMemory(MemoryManager& memoryManager, unsigned int startAddress, unsigned int numPages);
    void initializePageTable();
    vector<pair<string, string>> getPhysicalPages(unsigned int logicalPageStart, unsigned int numPages, MemoryManager& memManager);
    unsigned long long getAllocatedPhysicalMemory() const { return allocatedPhysicalMemory; }

    // New getter functions for outer and inner allocations
    unsigned long long getOuterPageTableMemory() const { return outerPageTableMemory; }
    unsigned long long getInnerPageTableMemory() const { return innerPageTableMemory; }

private:
    unsigned long long allocatedPhysicalMemory;
    string TaskID;
    int pageType; // 0: map, 1: single level, 2: two-level
    unordered_map<unsigned int, std::string> pageTable;
    unsigned long long logicalMemorySize;
    unsigned int pageSize;
    static constexpr unsigned int UNASSIGNED_PAGE = numeric_limits<unsigned int>::max();
    vector<string> SingleLevelPageTable;
    vector<vector<Frame*>> TwoLevelPageTable;
    Performance& performance;

    // New member variables to track outer and inner page table allocations
    unsigned long long outerPageTableMemory;
    unsigned long long innerPageTableMemory;

    // Constants for two-level page table
    static const int FIRST_LEVEL_BITS = 10;
    static const int SECOND_LEVEL_BITS = 10;
    static const int FIRST_LEVEL_ENTRIES = 1 << FIRST_LEVEL_BITS;
    static const int SECOND_LEVEL_ENTRIES = 1 << SECOND_LEVEL_BITS;

    // Helper methods for two-level page table
    void initializeTwoLevelPageTable();
    Frame* getTwoLevelPageEntry(unsigned int logicalPage);
    void setTwoLevelPageEntry(unsigned int logicalPage, const string& physicalPage);
};

#endif
