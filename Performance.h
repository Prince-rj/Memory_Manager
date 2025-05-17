#pragma once

#include <chrono>
#include <unordered_map>
#include <string>
#include "config.h"
using namespace std;

class Performance {
public:
    Performance();
    void startTimer();
    double endTimer();

    //records the memory used by the page table for a specific task
    void recordPageTableMemory(const std::string& taskId, unsigned long long memory);

    //records the physical memory allocated for a specific task
    void recordPhysicalMemoryAllocation(const std::string& taskId, unsigned long long memory);

    //records the memory used by the outer page table for a specific task
   void recordOuterPageTableMemory(const std::string& taskId, unsigned long long memory);
    
   //records the memory used by the inner page table for a specific task
   void recordInnerPageTableMemory(const std::string& taskId, unsigned long long memory);


    
    //records a page hit for a specific task
    void recordPageHit(const string& taskId); 
     //records a page miss for a specific task
     
    void recordPageMiss(const string& taskId); 
    void printResults();

private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::unordered_map<string, unsigned long long> pageTableMemory;
    std::unordered_map<string, unsigned long long> physicalMemoryAllocated;
    std::unordered_map<string, unsigned long long> pageHits;
    std::unordered_map<string, unsigned long long> pageMisses;
};
