#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include "csim.h"

// Global functions to keep track of total hit, miss, and evict count.
int hitCount = 0;
int missCount = 0;
int evictCount = 0;


// Main function ***********************************************************************************************************
int main(int argc, char *argv[]) {
    // Get the arguments from command line and store in struct
    ArgumentStruct argStruct = get_args(argc, argv);
    // Validate and open file for reading
    FILE *tracefile = get_and_validate_tracefile(argStruct.tracefile);
    // Calculate cache info based on command line arguments and store in struct
    CacheInfo cacheInfo = get_cache_info(argStruct);
    // Create a struct based on the cache information from above
    Cache cache = create_cache(cacheInfo);
    // Keep track of trace address array length for iteration purposes and get the addresses
    // from trace file as TraceAddress struct and store in an array
    size_t traceArrLen = 0;
    TraceAddress *traceArr = get_trace_arr(tracefile, cacheInfo, &traceArrLen);
    // Simulate the cache given the cache, its information, and trace
    simulate_cache(traceArr, traceArrLen, cache, cacheInfo, argStruct.verbose);
    // Print output and free allocated space
    print_and_free(&traceArr, traceArrLen, &cache, cacheInfo, &tracefile);

    // Exit program
    return EXIT_SUCCESS;
}


// Program functions *******************************************************************************************************
// Function to create a structure of arguments given by used
ArgumentStruct get_args(int argCount, char *argArr[]) {
    // Initialize struct values
    ArgumentStruct args;
    args.verbose = 0;
    args.setIndexBits = 0;
    args.associativity = 0;
    args.blockBits = 0;
    args.tracefile = NULL;

    // Integer where we store option
    int opt;
    while ((opt = getopt(argCount, argArr, "hvs:E:b:t:")) != -1) {
        // Switch case on integer option. Will use the argument as switch case
        // and assign the struct member the correct value
        switch (opt) {
            // If h, print usage and exit
            case 'h':
                print_usage_exit();
            // Set verbose to 1 if v
            case 'v':
                args.verbose = 1;
                break;
            // Get set index bits
            case 's':
                args.setIndexBits = atoi(optarg);
                break;
            // Get associativity
            case 'E':
                args.associativity = atoi(optarg);
                break;
            // Get block bits
            case 'b':
                args.blockBits = atoi(optarg);
                break;
            // Get file path
            case 't':
                args.tracefile = optarg;
                break;
            // None given, print usage and exit
            default:
                print_usage_exit();
        }
    }

    // Make sure that s, E, b, and t is given to us by user
    validate_args(args);
    return args;
}

// Function to validate and return the tracefile file pointer
FILE *get_and_validate_tracefile(char *tracefileString) {
    FILE *tracefile;
    // If we can not open the given path to trace file, then print error an exit
    if (!(tracefile = fopen(tracefileString, "r"))) {
        printf("./csim: cannot open \"%s\": No such file or directory\n", tracefileString);
        exit(EXIT_FAILURE);
    }
    return tracefile;
}

// Calculate the cache information given by the user from the arguments given,
// insert into struct, then return the struct
CacheInfo get_cache_info(ArgumentStruct args) {
    CacheInfo cacheInfo;
    cacheInfo.setIndexBits = args.setIndexBits;
    // S = 2^s, we are given s
    cacheInfo.sets = pow(2, args.setIndexBits);
    cacheInfo.blockBits = args.blockBits;
    // B = 2^b, we are given b
    cacheInfo.bytesPerBlock = pow(2, args.blockBits);
    cacheInfo.linesPerSet = args.associativity;

    return cacheInfo;
}

// Create the cache based on the struct CacheInfo and return the cache struct
Cache create_cache(CacheInfo cacheInfo) {
    Cache cache;
    // Allocate memory for sets in cache with the size of a cache set
    cache.set = calloc(cacheInfo.sets, sizeof(CacheSet));
    // For each set that we have, allocate space for however many lines we have
    // for each set given by E
    for (int i = 0; i < cacheInfo.sets; i++) {
        // Calloc space for cache lines
        cache.set[i].line = calloc(cacheInfo.linesPerSet, sizeof(CacheLine));
        // Set queue length to 0, necessary for FIFO implementations
        cache.set[i].queueLength = 0;
        // Initialize tag and valid bit to -1 indicate it has not be used
        for (int j = 0; j < cacheInfo.linesPerSet; j++) {
            cache.set[i].line[j].tag = -1;
            cache.set[i].line[j].valid = -1;
        }
    }
    return cache;
}

// Open tracefile and create struct of TraceAddress, then add to array of structs of
// TraceAddress then return that struct
TraceAddress *get_trace_arr(FILE *tracefile, CacheInfo cacheInfo, size_t *traceArrLen) {
    // Get value inside address of traceArrLen
    size_t len = *traceArrLen;
    // Iniditalize array to null for realloc
    TraceAddress *addressTraceArr = NULL;
    // Create buffer array for reading
    char buf[BUFSIZ];

    // While we are not at the end of the file, keep reading lines into buf array
    while (fgets(buf, sizeof(buf), tracefile) != NULL) {
        // If it does not start with an I and is not a newline
        if (buf[0] != 'I' && buf[0] != '\n') {
            TraceAddress currentTrace;
            // Pointer to the beginning of readline + 1 char
            char *stringp = buf + 1;

            // Token holds the first string separated by a space, store instruction
            // into struct
            char *token = strsep(&stringp, " ");
            currentTrace.instruction = token[0];

            // Get string delimited by comma, the address, then store it as a string
            // and store the actual integer value of string as hex
            token = strsep(&stringp, ",");
            currentTrace.addressString = strdup(token);
            currentTrace.address = (uint64_t)strtoq(token, NULL, 16);

            // Get size given by trace
            token = strsep(&stringp, "\n");
            currentTrace.size = atoi(token);

            // Call set_bit_mask to mask off where the block, set, and tag bits are located and
            // then return them as integers
            currentTrace.blockOffset = set_bit_mask(cacheInfo.blockBits, 0, currentTrace.address);
            currentTrace.setIndex = set_bit_mask(cacheInfo.setIndexBits, cacheInfo.blockBits, currentTrace.address);
            currentTrace.tag = set_bit_mask((64-(cacheInfo.blockBits+cacheInfo.setIndexBits)), (cacheInfo.blockBits+cacheInfo.setIndexBits), currentTrace.address);

            // Allocate space for 1 more TraceAddress struct in the address trace array then
            // insert new address
            addressTraceArr = realloc(addressTraceArr, sizeof(TraceAddress) * (len+1));
            addressTraceArr[len] = currentTrace;
            len++;
        }
    }
    *traceArrLen = len;
    return addressTraceArr;
}

// Function to start simulation of addresses and instructions within the tracefile
void simulate_cache(TraceAddress *traceArr, size_t traceArrLen, Cache cache, CacheInfo cacheInfo, int verbose) {
    // For all addresses in the trace file, perform a load/store instruction. Since load/store do
    // not actually load or store values, just need to determine if hit, miss, or evict.
    // Modify is a load followed by a store, so performing load/store will simulate a modify.
    // Load and store will always store in cache since the data is needed.
    for (int i = 0; i < traceArrLen; i++) {
        TraceAddress currentAddress = traceArr[i];
        // If verbose flag is set, print information about current address
        if (verbose == 1) {
            printf("%c %s,%i ", currentAddress.instruction, currentAddress.addressString, currentAddress.size);
        }
        // If instruction is not a modify, then perform load/store instruction
        if (currentAddress.instruction != 'M') {
            perform_ls(currentAddress, &cache, cacheInfo, verbose);
            // Simple function to print string incase verbose flag is on
            simple_verbose_print(verbose, "\n");
        } 
        else {
            // Perform load/store twice to simulate modify
            for (int j = 0; j < 2; j++) {
                perform_ls(currentAddress, &cache, cacheInfo, verbose);
            }
            simple_verbose_print(verbose, "\n");
        }
    }
}

// Simulate a load and store and determine if hit or miss or eviction
void perform_ls(TraceAddress currentAddress, Cache *cache, CacheInfo cacheInfo, int verbose) {
    Cache c = *cache;
    // Get index of set
    int setIndex = currentAddress.setIndex;

    // If there is nothing in the set, then cold miss, insert in first line of
    // the set
    if (c.set[setIndex].queueLength == 0) {
        missCount++;
        // If verbose flag on, print "miss "
        simple_verbose_print(verbose, "miss ");
        // Insert address into cache at line 0
        cache_insert(cache, currentAddress, 0);
        return;
    }
    // Else not a cold miss
    else {
        // Go through all lines in set and determine if hit
        for (int i = 0; i < cacheInfo.linesPerSet; i++) {
            // If current trace tag match the tag on line, then return a hit
            if (currentAddress.tag == c.set[setIndex].line[i].tag) {
                hitCount++;
                simple_verbose_print(verbose, "hit ");
                return;
            }
        }
        // If the queueLength of the cache is above the number of lines per set, then that
        // means that we have filled the set. queueLength is incremented everytime we store
        // a trace in the cache
        if (c.set[setIndex].queueLength >= cacheInfo.linesPerSet) {
            missCount++;
            evictCount++;
            simple_verbose_print(verbose, "miss eviction ");
            // Integer to initialize to find the lowest queueNumber in cache
            int compareInt = c.set[setIndex].line[0].queueNumber;
            // Integer for index to evict
            int indexToEvict = 0;
            // Linear search through set to find lowest queueNumber and set index i to
            // indexToEvict
            for (int i = 0; i < cacheInfo.linesPerSet; i++) {
                if (c.set[setIndex].line[i].queueNumber < compareInt) {
                    compareInt = c.set[setIndex].line[i].queueNumber;
                    indexToEvict = i;
                }
            }
            // Insert trace address here at indexToEvict
            cache_insert(cache, currentAddress, indexToEvict);
            return;
        }
        // Else, set is not full
        else {
            missCount++;
            simple_verbose_print(verbose, "miss ");
            // Find theh first line that has ann unset valid bit
            for (int i = 0; i < cacheInfo.linesPerSet; i++) {
                if (c.set[setIndex].line[i].valid == -1) {
                    // Insert address at first valid line
                    cache_insert(cache, currentAddress, i);
                    return;
                }
            }
        }
    }
}


// Helper functions ********************************************************************************************************
// Function to insert trace at line in set of cache
void cache_insert(Cache *cache, TraceAddress address, int index) {
    Cache c = *cache;
    // Set valid bit to 1
    c.set[address.setIndex].line[index].valid = 1;
    // Set tag to tag of address
    c.set[address.setIndex].line[index].tag = address.tag;
    // Set queueNumber to + 1 of queueLength
    c.set[address.setIndex].line[index].queueNumber = c.set[address.setIndex].queueLength + 1;
    // Increment set's queueLength
    c.set[address.setIndex].queueLength++;
}

// Function to mask bits of size x bits that is offset from the right.
// For example: we want the middle 2 bits from 0101, it will return 10.
int set_bit_mask(int numBits, int shift, uint64_t address) {
    // Initialize to be binary data = 00001
    int setBits = 1;
    // Shift for how many number of bits we want to mask, then set the lowest order bit to 
    // 1, then keep shifting. If we want to have 3 bits of 1, it will look like this:
    // 1 << 1 = 10 then 10 | 1 = 11 then 11 << 1 = 110 then 110 | 1 = 111... etc.
    for (int i = 0; i < numBits-1; i++) {
        setBits <<= 1;
        setBits |= 1;
    }
    // Shift right to move the group of bits set to 1 to specified position.
    // Ex: 111 << 3 = 111000, to mask the left 3 bits
    setBits <<= shift;
    // Get those bits from the address
    uint64_t val = address & setBits;
    // Shift back same amount as shift
    val >>= shift;
    // Return
    return (int)val;
}

// Validate that the s, E, b, and tracefiles are given by the user
void validate_args(ArgumentStruct args) {
    if (args.setIndexBits == 0 || args.associativity == 0 || args.blockBits == 0 || args.tracefile == NULL) {
        print_usage_exit();
    }
}

// Print usage then exit program
void print_usage_exit() {
    printf("Usage: csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
    exit(EXIT_FAILURE);
}

// Print string only if verbose flag set
void simple_verbose_print(int verbose, char *string) {
    if (verbose == 1) {
        printf("%s", string);
    }
}

// Print result of simulation then free the spaces that we have allocated
void print_and_free(TraceAddress **traceArr, size_t traceArrLen, Cache *cache, CacheInfo cacheInfo, FILE **tracefile) {
    TraceAddress *t = *traceArr;
    Cache c = *cache;
    FILE *f = *tracefile;

    printf("hits:%i misses:%i evictions:%i\n", hitCount, missCount, evictCount);
    // Free all strings of addressString since we used strdup which uses an alloc
    for (int i = 0; i < traceArrLen; i++) {
        free(t[i].addressString);
    }
    // Free all lines in eaach set since we allocated memory for lines
    for (int i = 0; i < cacheInfo.sets; i++) {
        free(c.set[i].line);
    }
    // Free the set
    free(c.set);
    // Free the tracearray
    free(t);
    // Close the file pointer
    fclose(f);
}
