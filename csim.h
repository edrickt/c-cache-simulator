// Structs *****************************************************************************************************************
// Holds arguments from command line
typedef struct ArgumentStruct {
    int verbose;
    int setIndexBits;
    int associativity;
    int blockBits;
    char *tracefile;
} ArgumentStruct;

// Holds information about cache size and bits
typedef struct CacheInfo {
    int setIndexBits;
    int linesPerSet;
    int blockBits;
    int sets;
    int bytesPerBlock;
} CacheInfo;

// A struct to simulate a cache line, does not need offset since we dont alter data
typedef struct CacheLine {
    int valid;
    int tag;
    // Newest lines added are always given queueNumber + 1 that is present in the line array
    int queueNumber; 
} CacheLine;

// A struct to simulate a set
typedef struct CacheSet {
    // An array of lines for associativity purposes
    struct CacheLine *line;
    // Will keep track of number of lines added to set overtime. Will assign queueLength + 1 to every
    // newn line then increment it. Evict the linen with the lowest queue number.
    int queueLength;
} CacheSet;

// Cache to hold array of sets
typedef struct Cache {
    struct CacheSet *set;
} Cache;

// A struct to hold the information from the trace file calculated using the cache info struct
typedef struct TraceAddress {
    char instruction;
    uint64_t address;
    char *addressString;
    int tag;
    int setIndex;
    int blockOffset;
    int size;
} TraceAddress;


// Function prototypes *****************************************************************************************************
ArgumentStruct get_args(int argCount, char *argArr[]);
void validate_args(ArgumentStruct args);
void print_usage_exit();
CacheInfo get_cache_info(ArgumentStruct args);
Cache create_cache(CacheInfo cacheInfo);
FILE *get_and_validate_tracefile(char *tracefile);
TraceAddress *get_trace_arr(FILE *tracefile, CacheInfo cacheInfo, size_t *traceArrLen);
int set_bit_mask(int numBits, int shift, uint64_t address);
void simulate_cache(TraceAddress *traceArr, size_t traceArrLen, Cache cache, CacheInfo cacheInfo, int verbose);
void perform_ls(TraceAddress currentAddress, Cache *cache, CacheInfo cacheInfo, int verbose);
void simple_verbose_print(int verbose, char *string);
void cache_insert(Cache *cache, TraceAddress address, int index);
void print_and_free(TraceAddress **traceArr, size_t traceArrLen, Cache *cache, CacheInfo cacheInfo, FILE **tracefile);
