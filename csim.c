#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

// Names: Chris Couto and Adam Beckwith  
// ID #1: ccouto@cs.hmc.edu
// ID #2: abeckwith@cs.hmc.edu

unsigned int ADDRESS_SIZE = 64;

unsigned int hit_count;
unsigned int miss_count;
unsigned int eviction_count;

// FLAGS (CHARACTER 'BOOLS')

char hit = 0;
char miss = 0;
char evict = 0;

char helpOn = 0;
char verboseOn = 0;

// PARSING DATA

char opt;
char * traceFile;
FILE *ftrace;

unsigned int blockBits;
unsigned int eSets;
unsigned int setInBits;
unsigned int tagSize;

unsigned int clockLRU = 0;

char operation;
long long unsigned address;
unsigned int size;

/**
    * \brief Data structure for a line in the cache. Tag stores the address tag, time stores the clock time for use with LRU, 
    *        valid stores the valid bit
**/
struct line {
    long long unsigned tag;
    unsigned int time;
    char valid;
};

struct line** cache;

/**
    * \brief Allocates memory for our cache data structure
    * \param setInBits  Number of set bits (s)  
    * \param eSets      Number of lines in each set (E)
    * \param blockBits  Number of block bits (b)
**/
void getCache(int setInBits, int eSets, int blockBits) {
    cache = (struct line **) malloc((1<<setInBits)*sizeof(struct line *));
    // Allocate array of line structs for each set in cache
    for (int i = 0; i < ((1<<setInBits)); ++i) {
       cache[i] = (struct line *) malloc(eSets*sizeof(struct line));
    }
 }

/**
    * \brief Deallocates memory for our cache data structure
**/
void removeCache() {
    // Deallocate array of line structs for each set in cache
    for (int i = 0; i < (1<<setInBits); ++i) {
        free(cache[i]);
    }
    free(cache);
}

/**
    * \brief Determines hit, miss, or eviction for input address and adds tag and clock time to cache accordingly. 
    *        Parses input address and updates counters (clock, hit, miss, evict) as well
    * \param address  The 64-bit hexadecimal address being added to the cache
**/
void updateCache(long long unsigned address) {
    
    clockLRU++; // increment counter
    long long unsigned tval = address >> (blockBits+setInBits);
    long long unsigned sval = (address << (tagSize)) >> (blockBits+tagSize);
    int firstEmpty = -1;
    // Check all lines in desired set for a hit in the cache
    for (int i=0; i < eSets; ++i) {
        // Looks if the line is valid and the tag matches, otherwise checks if it's the first empty line in the set
        if (cache[sval][i].valid) {
            if (cache[sval][i].tag == tval) {
                // hit!
                cache[sval][i].time = clockLRU;
                hit = 1;
                hit_count++;
                break; 
            }
        } else if (firstEmpty == -1) {
            firstEmpty = i;
        }
    }
    // Tag not in set, decide if we have room or not to add
    if ((firstEmpty == -1) && (!hit)) {
        // Miss and evict!
        miss_count++;
        miss = 1;
        eviction_count++;
        evict = 1; 
        int lowestTime = cache[sval][0].time;
        int lowestLine = 0;
        // Find line in set with lowest clock time for LRU eviction
        for (int i = 1; i < eSets; ++i) {
            if (cache[sval][i].time < lowestTime) {
                lowestTime = cache[sval][i].time;
                lowestLine = i;
            }
        }
        cache[sval][lowestLine].tag = tval;
        cache[sval][lowestLine].time = clockLRU;
    } else if ((firstEmpty >= 0) && (!hit)) {
        // Miss and don't evict!
        miss_count++;
        miss = 1;
        cache[sval][firstEmpty].valid = 0x1;
        cache[sval][firstEmpty].tag = tval;
        cache[sval][firstEmpty].time = clockLRU;
    }
}

int main(int argc, char * argv[])
{
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
       switch(opt)
       {
        // Get info from command line
        case 'h':  // turns on help 
            helpOn = 1;
            break;
        case 'v':
            verboseOn = 1;  // prints with each miss/hit of the cache
            break;
        case 's':
            setInBits = atoi(optarg); 
            break;
        case 'E':
            eSets = atoi(optarg);
            break;
        case 'b':
            blockBits = atoi(optarg);
            break;
        case 't':
            traceFile = optarg;
            break;
       }
    }
    if(helpOn) {
        printf("This is a program that simulates a memory cache off of Valgrind trace files.\n");
    }

    getCache(setInBits, eSets, blockBits);

    tagSize = ADDRESS_SIZE - (setInBits + blockBits); 

    ftrace = fopen(traceFile, "r");
    // Scans each line in input trace file and processes it through cache
    while (fscanf(ftrace, " %c  %llx, %u", &operation, &address, &size) == 3) {
        // Finds if operation is load or store (0x4c is ASCII for L, 0x53 is ASCII for S)
        if (operation == 0x4c || operation == 0x53) {
            updateCache(address);
        }
        // Finds if operation is modify (0x4d is ASCII for M)
        if (operation == 0x4d) {
            updateCache(address);
            updateCache(address);
        }
        if (verboseOn) {
            printf("%c %llx, %u ", operation, address, size);
                
            (miss)? (printf("miss ")):(printf(" "));
            (evict)? (printf("eviction ")):(printf(" "));
            (hit)? (printf("hit ")):(printf(" "));
            printf("\n");
        }
        hit = 0;  // reset character 'bools'
        miss = 0;
        evict = 0;
    }
    removeCache();
    printSummary(hit_count, miss_count, eviction_count);

    return 0;
}