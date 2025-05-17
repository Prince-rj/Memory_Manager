
#include "TaskManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <math.h>
using namespace std;

const string UNASSIGNED_PAGE_STR = "0xFFFFFFFF";

//converts address to hexadecimal using th ostringstream 
string addressToHex(unsigned int address,unsigned long long logicalMemorySize) {
    ostringstream stream;
    stream << "0x" << setw(ceil(log2(logicalMemorySize)/4)) << setfill('0') << hex << address;
    return stream.str();
}



//constructor
Task::Task(string TaskID, unsigned long long logicalMemorySize, MemoryManager& memoryManager, unsigned int pageSize, int pageType, Performance &perf)
    : pageType(pageType), TaskID(TaskID), logicalMemorySize(logicalMemorySize), pageSize(pageSize), performance(perf), allocatedPhysicalMemory(0) {
    initializePageTable();
    
    const unsigned int pageTableEntrySize =  ceil(log2(logicalMemorySize)/8);
    unsigned int numPageTableEntries = logicalMemorySize / pageSize;
    unsigned int pageTableSize = numPageTableEntries * pageTableEntrySize;
    unsigned int numPagesInPageTable = (pageTableSize + pageSize - 1) / pageSize;
    //we when we have map we need to make sure that the page table is storing 2 values
    if (pageType == 2) {
        auto pageTablePages = memoryManager.allocatePages(numPagesInPageTable, true);
         //For two-level page table
         unsigned int outerTableSize = OUTER_PAGE_TABLE_ENTRIES * pageTableEntrySize;
         unsigned int innerTableSize = pageTableSize - outerTableSize;
         performance.recordOuterPageTableMemory(TaskID, outerTableSize);
         performance.recordInnerPageTableMemory(TaskID, innerTableSize);
    } else if(pageType==1) {
        auto pageTablePages = memoryManager.allocatePages(numPagesInPageTable, true);
        performance.recordPageTableMemory(TaskID, pageTableSize);
    }
}


//initialize pagetable as according to page type.
void Task::initializePageTable() {
    unsigned int numPages = logicalMemorySize / pageSize;
     if (pageType == 1) {
        SingleLevelPageTable.resize(numPages, UNASSIGNED_PAGE_STR);
    } else if (pageType == 2) {
        // TwoLevelPageTable.resize(OUTER_PAGE_TABLE_ENTRIES, vector<string>(INNER_PAGE_TABLE_ENTRIES, UNASSIGNED_PAGE_STR));
        initializeTwoLevelPageTable();
    }
}

//initialize pagetable as according to page type.
void Task::initializeTwoLevelPageTable() {
    TwoLevelPageTable.resize(FIRST_LEVEL_ENTRIES, vector<Frame*>(0));
    // Assume each outer page table entry occupies one page
    unsigned int outerPagesAllocated = FIRST_LEVEL_ENTRIES; // Each outer entry could be a separate page
    outerPageTableMemory += outerPagesAllocated * pageSize; // Assuming pageSize is available
    // Allocate outer page table memory (assuming MemoryManager handles this)
    // If outer page tables are stored as multiple pages, adjust accordingly
}


Frame* Task::getTwoLevelPageEntry(unsigned int logicalPage) {
    unsigned int firstLevelIndex = (logicalPage >> SECOND_LEVEL_BITS) & ((1 << FIRST_LEVEL_BITS) - 1);
    unsigned int secondLevelIndex = logicalPage & ((1 << SECOND_LEVEL_BITS) - 1);

    if (firstLevelIndex >= TwoLevelPageTable.size() || TwoLevelPageTable[firstLevelIndex].empty()) {
        return nullptr; // Second-level table doesn't exist yet
    }

    return TwoLevelPageTable[firstLevelIndex][secondLevelIndex];
}

void Task::setTwoLevelPageEntry(unsigned int logicalPage, const string& physicalPage) {
    unsigned int firstLevelIndex = (logicalPage >> SECOND_LEVEL_BITS) & ((1 << FIRST_LEVEL_BITS) - 1);
    unsigned int secondLevelIndex = logicalPage & ((1 << SECOND_LEVEL_BITS) - 1);

    if (firstLevelIndex >= TwoLevelPageTable.size()) {
        cerr << "First level index out of range." << endl;
        return;
    }

    if (TwoLevelPageTable[firstLevelIndex].empty()) {
        // Allocate a new second-level page table
        TwoLevelPageTable[firstLevelIndex].resize(SECOND_LEVEL_ENTRIES, nullptr);
        // Track memory allocation for inner page tables
        unsigned int innerPagesAllocated = SECOND_LEVEL_ENTRIES; // Assuming each inner table occupies one page
        innerPageTableMemory += innerPagesAllocated * pageSize; // Assuming pageSize is available
        // Allocate inner page table memory (assuming MemoryManager handles this)
    }

    if (!TwoLevelPageTable[firstLevelIndex][secondLevelIndex]) {
        TwoLevelPageTable[firstLevelIndex][secondLevelIndex] = new Frame();
    }

    TwoLevelPageTable[firstLevelIndex][secondLevelIndex]->physicalAddress = physicalPage;
}


//we allocate the physical memory to page table using th allocatePages function that is in the Memory manager module.
vector<string> Task::allocateMemory(MemoryManager& memoryManager, unsigned int startAddress, unsigned int numPages) {
    unsigned int startPage = startAddress / pageSize;
    vector<string> physicalPagesHex = memoryManager.allocatePages(numPages, false);
    if (physicalPagesHex.empty()) {
        cerr << "Memory allocation failed for task " << TaskID << endl;
        return physicalPagesHex;
    }

    unsigned long long newlyAllocatedMemory = numPages * pageSize;
    allocatedPhysicalMemory += newlyAllocatedMemory;
    performance.recordPhysicalMemoryAllocation(TaskID, newlyAllocatedMemory);
    int mapsizebefore=0;
    if(pageType==0)mapsizebefore=pageTable.size();
    for (unsigned int i = 0; i < numPages; ++i) {
        unsigned int logicalPage = startPage + i;
        unsigned int physicalPageIndex = i;
        if (pageType == 0) {
            pageTable[logicalPage] = physicalPagesHex[physicalPageIndex];
        } else if (pageType == 1) {
            SingleLevelPageTable[logicalPage] = physicalPagesHex[physicalPageIndex];
        } else if (pageType == 2) {
            setTwoLevelPageEntry(logicalPage, physicalPagesHex[physicalPageIndex]);
        }
    }
    if(pageType==0){
        int mapsizeafter=pageTable.size();
        int pageTableEntrySize=2*ceil(log2(logicalMemorySize)/8);
        int pageTableSize=pageTableEntrySize*mapsizeafter;
        performance.recordPageTableMemory(TaskID, pageTableSize);
        int sizeincr=mapsizeafter-mapsizebefore;
        int pageTableSizeincr=pageTableEntrySize*sizeincr;
        unsigned int numPagesInPageTable = (pageTableSizeincr + pageSize - 1) / pageSize;
        auto pageTablePages = memoryManager.allocatePages(numPagesInPageTable, true);

    }
    return physicalPagesHex;
}

vector<pair<string, string>> Task::getPhysicalPages(unsigned int logicalPageStart, unsigned int numPages, MemoryManager& memManager) {
    vector<pair<string, string>> result(numPages, {UNASSIGNED_PAGE_STR, UNASSIGNED_PAGE_STR});
    int pageHits = 0;
    unsigned int endPage = logicalPageStart/pageSize + numPages;

   // iterate through the logical pages to find assigned pages
    for (unsigned int page = logicalPageStart/pageSize; page < endPage; ++page) {
        string physicalPage = UNASSIGNED_PAGE_STR;
        if (pageType == 0) {
            auto it = pageTable.find(page);
            if (it != pageTable.end() && it->second != UNASSIGNED_PAGE_STR) {
                physicalPage = it->second;
            }
        } else if (pageType == 1) {
            if (page < SingleLevelPageTable.size() && SingleLevelPageTable[page] != UNASSIGNED_PAGE_STR) {
                physicalPage = SingleLevelPageTable[page];
            }
        } else if (pageType == 2) {
            Frame* frame = getTwoLevelPageEntry(page);
            if (frame) {
                physicalPage = frame->physicalAddress;
            }
        }
        if (physicalPage != UNASSIGNED_PAGE_STR) {
            // Page is assigned so count it as a page hit
            result[page - logicalPageStart/pageSize] = {physicalPage, addressToHex(page*pageSize,logicalMemorySize)};
            ++pageHits;
            performance.recordPageHit(TaskID);
        }
    }

   //if we need more pages allocate and map them
    if (pageHits < numPages) {
        unsigned int pagesToAllocate = numPages - pageHits;
        vector<string> newPages = allocateMemory(memManager, logicalPageStart + pageHits * pageSize, pagesToAllocate);

        if (newPages.size() == pagesToAllocate) { 
            for (unsigned int i = 0; i < newPages.size(); ++i) {
                performance.recordPageMiss(TaskID);
                unsigned int logicalPage = logicalPageStart/pageSize + pageHits + i;
                result[pageHits + i] = {newPages[i], addressToHex(logicalPage*pageSize,logicalMemorySize)};
            }
            pageHits += newPages.size();
        } else {
            cerr << "Failed to allocate pages." << endl;
        }
    }

    return result;
}
Task::~Task() {
    if (pageType == 2) {
        for (auto& secondLevelTable : TwoLevelPageTable) {
            if (!secondLevelTable.empty()) {
                for (auto& frame : secondLevelTable) {
                    delete frame;  // Delete dynamically allocated frames
                }
            }
        }
    }
}