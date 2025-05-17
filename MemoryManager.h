#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <string>

class MemoryManager {
public:
    MemoryManager(unsigned long long memorySize, unsigned int pageSize);

    std::vector<std::string> allocatePages(unsigned int numPages, bool isPageTable);
    unsigned int getFreePageCount() const;

private:
    unsigned long long memorySize;
    unsigned int pageSize;
    unsigned int numFrames;
    std::vector<int> memory;
};

#endif 
