#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>
#include "TaskManager.h"  
#include "MemoryManager.h" 
#include "Performance.h"
#include "config.h"

using namespace std;

// Assuming config.h defines TOTAL_VIRTUAL_MEMORY, TOTAL_PHYSICAL_MEMORY, PAGE_SIZE

unsigned long long virtualMemorySize = TOTAL_VIRTUAL_MEMORY;
unsigned long long MemorySize = TOTAL_PHYSICAL_MEMORY;
unsigned int pageSize = PAGE_SIZE; 
unordered_map<string, Task> taskMap;
MemoryManager memManager(MemorySize, pageSize);
Performance performance;


string formatMemory_Size(unsigned long long bytes) {
    std::ostringstream formattedSize;
    formattedSize << std::fixed << std::setprecision(2);

    if (bytes >= 1024ull * 1024 * 1024) {
        formattedSize << (bytes / (1024.0 * 1024 * 1024)) << " GB";
    } else if (bytes >= 1024ull * 1024) {
        formattedSize << (bytes / (1024.0 * 1024)) << " MB";
    } else if (bytes >= 1024ull) {
        formattedSize << (bytes / 1024.0) << " KB";
    } else {
        formattedSize << bytes << " bytes";
    }

    return formattedSize.str();
}




string combinePhysicalAddress(string& frameAddressStr, unsigned int logicalAddress) {
    unsigned int frameAddress = 0;
    istringstream(frameAddressStr) >> hex >> frameAddress;

    unsigned int offsetBits = static_cast<unsigned int>(log2(pageSize));
    unsigned int offsetMask = (1 << offsetBits) - 1;

    unsigned int offset = logicalAddress & offsetMask;

    unsigned int fullPhysicalAddress = (frameAddress & ~offsetMask) | (offset & offsetMask);

    ostringstream resultStream;
    resultStream << "0x" << setw(8) << setfill('0') << hex << fullPhysicalAddress;
    return resultStream.str();
}

void processTraceFile(const string& filename, int pageType) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string taskId, addressStr, sizeStr;

        if (getline(iss, taskId, ':') && getline(iss, addressStr, ':') && getline(iss, sizeStr)) {
            // Trim leading spaces
            taskId = taskId.substr(taskId.find_first_not_of(' '));
            addressStr = addressStr.substr(addressStr.find_first_not_of(' '));
            sizeStr = sizeStr.substr(sizeStr.find_first_not_of(' '));

            unsigned int address;
            try {
                address = stoul(addressStr, nullptr, 16);
            } catch (const invalid_argument& e) {
                cerr << "Invalid address format: " << addressStr << endl;
                continue;
            } catch (const out_of_range& e) {
                cerr << "Address out of range: " << addressStr << endl;
                continue;
            }

            unsigned int size = 0;
            try {
                size_t unitPos = sizeStr.find_first_of("KM");
                string sizePart = sizeStr.substr(0, unitPos);
                size = stoul(sizePart);
                if (unitPos != string::npos) {
                    if (sizeStr[unitPos] == 'K') {
                        size *= 1024;
                    } else if (sizeStr[unitPos] == 'M') {
                        size *= 1024 * 1024;
                    }
                    else{
                        size = stoul(sizeStr);
                    }
                }
            } catch (const invalid_argument& e) {
                cerr << "Invalid size format: " << sizeStr << endl;
                continue;
            } catch (const out_of_range& e) {
                cerr << "Size out of range: " << sizeStr << endl;
                continue;
            }
            
            unsigned int numPages = (size + pageSize - 1) / pageSize;

            auto it = taskMap.find(taskId);
            if (it == taskMap.end()) {
                Task newTask(taskId, virtualMemorySize, memManager, pageSize, pageType, performance);
                it = taskMap.emplace(taskId, move(newTask)).first;
            }

            Task& task = it->second;
            vector<pair<string, string>> result = task.getPhysicalPages(address, numPages, memManager);
            string physical_addr = combinePhysicalAddress(result[0].first, address);
            cout << "Physical Address for " << addressToHex(address, virtualMemorySize) << " is " << physical_addr << endl;
             unsigned long long freeMemory = memManager.getFreePageCount() * pageSize;
            //  cout << "Free physical memory: " << formatMemory_Size(freeMemory) << endl;

            result.clear();
        }
    }
    file.close();
}

int main() {
    int pageType;
    cout << "Enter the Page Type:\n"
         << "0: Map as page Table\n"
         << "1: Single level page table\n"
         << "2: Two-level page table\n"
         << ": ";
    cin >> pageType;

  
    performance.startTimer();
    processTraceFile("trace_output.txt", pageType);
    double executionTime = performance.endTimer();

    string s = pageType == 0 ? "Map based page table" :
               pageType == 1 ? "Single level page table" :
               pageType == 2 ? "Two-level page table" :
               "Unknown page table type";


    cout << "\n Execution time for " << s << ": " << executionTime << " ms" << endl;



    // Displaying the page table memory usage
    // for (const auto& task : taskMap) {
    //     if (pageType == 2) { // Two-level page table
    //         unsigned int outerMemory = task.second.getOuterPageTableMemory();
    //         unsigned int innerMemory = task.second.getInnerPageTableMemory();
    //         cout << "Task ID: "  <<  task.first << endl;
    //         // " uses two-level page table."
    //         cout << "Outer page table memory: " << formatMemory_Size(outerMemory) << endl;
    //         cout << "Inner page table memory: " << formatMemory_Size(innerMemory) << endl;
    //     }
    //     // } else if (task.second.getPageType() == 1) { // Single-level page table
    //     //     unsigned int pageTableMemory = task.second.getPageTableMemory();
    //     //     cout << "Task ID: " << task.first << " uses single-level page table." << endl;
    //     //     cout << "Page table memory: " << formatMemory_Size(pageTableMemory) << endl;
    //     // }
    // }

    performance.printResults();

    unsigned long long freeMemory = memManager.getFreePageCount() * pageSize;
    cout << "Free physical memory: " << formatMemory_Size(freeMemory) << endl;
    cout << endl;
    return 0;
}
