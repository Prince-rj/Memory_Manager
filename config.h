//We define the configurations here the first we have the parameters like pagesize, virtual memory size
//physical memory size and num sections
#ifndef CONFIG_H
#define CONFIG_H


const int  OUTER_PAGE_TABLE_ENTRIES = 1024;  // Example value
const int INNER_PAGE_TABLE_ENTRIES = 1024;  // Example value 

const int PAGE_SIZE = 4*1024;  
const int NUM_SECTIONS = 5;  
const unsigned long long TOTAL_VIRTUAL_MEMORY = 4LL * 1024*1024*1024; 
const unsigned long long TOTAL_PHYSICAL_MEMORY = 4LL*1024*1024*1024;


//I am here taking the every section size as 1/5 of total virtual memory as nothing specific was provided
const int TEXT_SECTION_SIZE = 2*1024*1024;  
const int DATA_SECTION_SIZE = 50*1024;  
const int SHARED_LIB_SECTION_SIZE = 50*1024;
const int STACK_SECTION_SIZE = 2*1024*1024;  
const int HEAP_SECTION_SIZE = 2*1024*1024;  


//now starting from 0 I go all the way to the size of vitrual memory in sections and define the starting address
//for each of them
const long START_ADDRESS_TEXT = 0;
const long START_ADDRESS_DATA = START_ADDRESS_TEXT+TEXT_SECTION_SIZE;
const long START_ADDRESS_SHARED_LIB = START_ADDRESS_DATA +DATA_SECTION_SIZE;
const long START_ADDRESS_HEAP = START_ADDRESS_SHARED_LIB+SHARED_LIB_SECTION_SIZE;
const long START_ADDRESS_STACK = START_ADDRESS_HEAP + TOTAL_VIRTUAL_MEMORY/2;  

#endif

