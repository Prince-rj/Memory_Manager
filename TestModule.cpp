#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <pthread.h>
#include "config.h"
using namespace std;


struct ThreadData {
    int task_id;
    int traces_to_generate;
    ofstream *trace_file;
};

// Mutex for thread-safe file writing
pthread_mutex_t file_mutex;

int get_section_size_in_pages(int section_type) {
    int size_in_bytes = 0;
    switch (section_type) {
        case 0: size_in_bytes = rand() % (TEXT_SECTION_SIZE + 1); break;
        case 1: size_in_bytes = rand() % (DATA_SECTION_SIZE + 1); break;
        case 2: size_in_bytes = rand() % (SHARED_LIB_SECTION_SIZE + 1); break;
        case 3: size_in_bytes = rand() % (HEAP_SECTION_SIZE + 1); break;
        case 4: size_in_bytes = rand() % (STACK_SECTION_SIZE + 1); break;
    }
    int size_in_pages = (size_in_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    return (size_in_pages > 0) ? size_in_pages : 1; // Ensure at least 1 page
}

void* generate_trace_thread(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    int task = data->task_id;
    int traces_to_generate = data->traces_to_generate;
    ofstream &trace_file = *(data->trace_file);

    for (int i = 0; i < traces_to_generate; i++) {
        string task_id = "T" + to_string(task);

        int section_choice = rand() % NUM_SECTIONS;
        int section_size_in_pages = get_section_size_in_pages(section_choice);

        if (section_size_in_pages <= 0) {
            cerr << "Error: Invalid section size. Exiting..." << endl;
            return nullptr;
        }

        long base_address = 0;
        switch (section_choice) {
            case 0: base_address = START_ADDRESS_TEXT; break;
            case 1: base_address = START_ADDRESS_DATA; break;
            case 2: base_address = START_ADDRESS_SHARED_LIB; break;
            case 3: base_address = START_ADDRESS_HEAP; break;
            case 4: base_address = START_ADDRESS_STACK; break;
        }

        long address = base_address + (rand() % section_size_in_pages) * PAGE_SIZE;
        address = address & 0xFFFFFFFF;

        int remaining_size_in_section = (base_address + section_size_in_pages * PAGE_SIZE) - address;
        int max_size_in_kb = remaining_size_in_section / 1024;
        int size_in_kb = (rand() % max_size_in_kb + 1);

        pthread_mutex_lock(&file_mutex);  // Lock before writing to the file
        trace_file << task_id << ": 0x" << setw(8) << setfill('0') << hex << address << dec;
        if (size_in_kb / 1024 > 0) {
            trace_file << ":" << size_in_kb / 1024 << "MB\n";
        } else {
            trace_file << ":" << size_in_kb << "KB\n";
        }
        pthread_mutex_unlock(&file_mutex);  // Unlock after writing
    }
    return nullptr;
}

int main() {
    srand(1007);  // Initialize random seed once

    int num_tasks;
    cout << "Enter number of tasks: ";
    cin >> num_tasks;
    int num_traces;
    cout << "Enter no. of traces: ";
    cin >> num_traces;

    ofstream trace_file("trace_output.txt");

    if (trace_file.is_open()) {
        pthread_mutex_init(&file_mutex, nullptr);

        vector<pthread_t> threads(num_tasks);
        vector<ThreadData> thread_data(num_tasks);

        int traces_allocated = 0;
        for (int i = 0; i < num_tasks - 1; i++) {
            // Allocate a random number of traces to each task
            thread_data[i].traces_to_generate = rand() % (num_traces - traces_allocated);
            traces_allocated += thread_data[i].traces_to_generate;
            thread_data[i].task_id = i + 1;
            thread_data[i].trace_file = &trace_file;

            pthread_create(&threads[i], nullptr, generate_trace_thread, &thread_data[i]);
        }

        // Assign the remaining traces to the last task
        thread_data[num_tasks - 1].traces_to_generate = num_traces - traces_allocated;
        thread_data[num_tasks - 1].task_id = num_tasks;
        thread_data[num_tasks - 1].trace_file = &trace_file;

        pthread_create(&threads[num_tasks - 1], nullptr, generate_trace_thread, &thread_data[num_tasks - 1]);

        // Wait for all threads to complete
        for (int i = 0; i < num_tasks; i++) {
            pthread_join(threads[i], nullptr);
        }

        pthread_mutex_destroy(&file_mutex);
        trace_file.close();

        cout << "Trace file generated in trace_output.txt" << endl;
    } else {
        cerr << "Unable to open trace file." << endl;
    }

    return 0;
}