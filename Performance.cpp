#include "Performance.h"
#include <iostream>
#include <iomanip>

Performance::Performance() {}

void Performance::startTimer() 
{
startTime = std::chrono::high_resolution_clock::now();
}
double Performance::endTimer() 
{
auto endTime = std::chrono::high_resolution_clock::now();
return std::chrono::duration<double, std::milli>(endTime - startTime).count();
}
void Performance::recordPageTableMemory(const std::string& taskId, unsigned long long memory) 
{
pageTableMemory[taskId] += memory;
// Adds the specified memory to the total page table memory for the given taskId.
}
void Performance::recordPhysicalMemoryAllocation(const std::string& taskId, unsigned long long memory) 
{
physicalMemoryAllocated[taskId] += memory;
// Adds the specified memory to the total physical memory allocated for the given taskId.
}
void Performance::recordPageHit(const std::string& taskId) 
{
pageHits[taskId]++;
// Increments the page hit count for the specified taskId.
}
void Performance::recordPageMiss(const std::string& taskId) 
{
pageMisses[taskId]++;
// Increments the page miss count for the specified taskId.
}

void Performance::recordOuterPageTableMemory(const std::string& taskId, unsigned long long memory) 
{
    pageTableMemory[taskId] += memory;
    // Adds the specified memory to the total outer page table memory for the given taskId.
}

void Performance::recordInnerPageTableMemory(const std::string& taskId, unsigned long long memory) 
{
    pageTableMemory[taskId] += memory;
    // Adds the specified memory to the total inner page table memory for the given taskId.
}




//function to convert bytes to  (GB, MB, KB) accordingly
string formatMemorySize(unsigned long long bytes) 
{
    double size = bytes;
    std::string unit = " bytes";

    if (size > 1024 * 1024 * 1024) 
    {
        size /= (1024 * 1024 * 1024);
        unit = " GB";
    } 
    else if (size > 1024 * 1024)
    {
        size /= (1024 * 1024);
        unit = " MB";
    } else if (size > 1024) 
    {
        size /= 1024;
        unit = " KB";
    }

    ostringstream oss;
    oss << fixed << setprecision(2) << size << unit;
    return oss.str();
}














void Performance::printResults() 
{
std::cout << "Performance Results:\n";
std::cout << "Page Table Memory Usage:\n";
for (const auto& entry : pageTableMemory)
{
 std::cout << "  Task " << entry.first << ": " << formatMemorySize(entry.second) << "\n";
// Iterates through the pageTableMemory map and prints the memory usage for each task.
}
std::cout << "Physical Memory Allocated:\n";
for (const auto& entry : physicalMemoryAllocated) {
 std::cout << "  Task " << entry.first << ": " << formatMemorySize(entry.second) << "\n";
 // Iterates through the physicalMemoryAllocated map and prints the allocated memory for each task.
}

std::cout << "Page Hits :\n";

for (const auto& entry : pageHits) {
    std::cout << "  Task " << entry.first << ": Hits = " << entry.second<<endl;
}
std::cout << "Page Misses :\n";
for(const auto& entry : pageMisses){
    std::cout<<"Task :"<<entry.first<< "  Misses = " << entry.second<<endl;
}
    // << ", Hit Ratio = " << std::fixed << std::setprecision(2)
    // << (static_cast<double>(entry.second) / (entry.second + pageMisses[entry.first])) * 100 << "%\n";
    // Iterates through the pageHits map, prints the hits, misses, and calculates the hit ratio for each task.
    // The hit ratio is calculated as (hits / (hits + misses)) * 100 and is formatted to two decimal places.
}