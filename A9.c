/*****************************************************************************
 * Data Cache Simulator with LRU Replacement Policy
 * 
 * Author: Jake Spillers
 * Course: CDA3100 - Computer Organization
 * 
 * Description:
 *   Simulates a configurable set-associative cache with write-through and
 *   no-write-allocate policies. Implements LRU (Least Recently Used)
 *   replacement for cache misses.
 * 
 * Compilation:
 *   gcc -std=c99 -Wall -Wextra -O2 -o cache_simulator cache_simulator.c
 * 
 * Usage:
 *   ./cache_simulator < trace_file
 *   
 *   Requires trace.config file with format:
 *     Number of sets: <num>
 *     Set size: <num>
 *     Line size: <num>
 * 
 * Cache Policies:
 *   - Write-through: Writes always go to memory
 *   - No-write-allocate: Write misses don't load cache lines
 *   - LRU replacement: Evicts least recently used line on read misses
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_CACHE_SETS 8192
#define MAX_ASSOCIATIVITY 8
#define MAX_LINE_SIZE 64

/**
 * Cache configuration parameters
 */
typedef struct {
    int num_sets;        // Number of cache sets
    int associativity;   // Lines per set (set size)
    int line_size;       // Bytes per cache line
    int offset_bits;     // Number of bits for offset
    int index_bits;      // Number of bits for index
} CacheConfig;

/**
 * Cache line structure with LRU tracking
 */
typedef struct {
    int valid;           // Valid bit
    unsigned int tag;    // Tag bits
    unsigned int lru_counter; // For LRU replacement (higher = more recent)
} CacheLine;

/**
 * Statistics tracking structure
 */
typedef struct {
    int hits;
    int misses;
    int mem_reads;
    int mem_writes;
} CacheStats;

/**
 * Calculate log base 2 of a number
 */
int log2_int(int n) {
    int bits = 0;
    while (n > 1) {
        n >>= 1;
        bits++;
    }
    return bits;
}

/**
 * Initialize cache structure and configuration
 */
void init_cache(CacheLine ***cache, CacheConfig *config) {
    // Calculate bit field sizes
    config->offset_bits = log2_int(config->line_size);
    config->index_bits = log2_int(config->num_sets);
    
    // Allocate cache memory
    *cache = (CacheLine **)malloc(config->num_sets * sizeof(CacheLine *));
    if (*cache == NULL) {
        fprintf(stderr, "Error: Failed to allocate cache memory\n");
        exit(1);
    }
    
    for (int i = 0; i < config->num_sets; i++) {
        (*cache)[i] = (CacheLine *)malloc(config->associativity * sizeof(CacheLine));
        if ((*cache)[i] == NULL) {
            fprintf(stderr, "Error: Failed to allocate cache set memory\n");
            exit(1);
        }
        
        // Initialize all cache lines
        for (int j = 0; j < config->associativity; j++) {
            (*cache)[i][j].valid = 0;
            (*cache)[i][j].tag = 0;
            (*cache)[i][j].lru_counter = 0;
        }
    }
}

/**
 * Update LRU counters for a cache set
 */
void update_lru(CacheLine *set, int associativity, int accessed_way) {
    unsigned int accessed_counter = set[accessed_way].lru_counter;
    
    // Increment all counters that were more recent than accessed line
    for (int i = 0; i < associativity; i++) {
        if (set[i].valid && set[i].lru_counter > accessed_counter) {
            set[i].lru_counter--;
        }
    }
    
    // Set accessed line to most recent
    set[accessed_way].lru_counter = associativity - 1;
}

/**
 * Find LRU victim for replacement
 */
int find_lru_victim(CacheLine *set, int associativity) {
    int lru_way = 0;
    unsigned int min_counter = set[0].lru_counter;
    
    // Find invalid line first (cold miss)
    for (int i = 0; i < associativity; i++) {
        if (!set[i].valid) {
            return i;
        }
    }
    
    // Find LRU line
    for (int i = 1; i < associativity; i++) {
        if (set[i].lru_counter < min_counter) {
            min_counter = set[i].lru_counter;
            lru_way = i;
        }
    }
    
    return lru_way;
}

/**
 * Simulate a cache access
 */
void access_cache(CacheLine **cache, CacheConfig *config, char access_type,
                  unsigned int address, CacheStats *stats) {
    
    // Extract address components
    unsigned int offset = address & ((1 << config->offset_bits) - 1);
    unsigned int index = (address >> config->offset_bits) & ((1 << config->index_bits) - 1);
    unsigned int tag = address >> (config->offset_bits + config->index_bits);
    
    // Search for tag in the set
    int hit = 0;
    int hit_way = -1;
    
    for (int i = 0; i < config->associativity; i++) {
        if (cache[index][i].valid && cache[index][i].tag == tag) {
            hit = 1;
            hit_way = i;
            break;
        }
    }
    
    // Handle read access
    if (access_type == 'R' || access_type == 'r') {
        if (hit) {
            stats->hits++;
            update_lru(cache[index], config->associativity, hit_way);
        } else {
            stats->misses++;
            stats->mem_reads++;
            
            // Find victim and replace
            int victim_way = find_lru_victim(cache[index], config->associativity);
            cache[index][victim_way].valid = 1;
            cache[index][victim_way].tag = tag;
            update_lru(cache[index], config->associativity, victim_way);
        }
        
        printf("%c %08x %x %x %x %s %d\n",
               access_type, address, tag, index, offset,
               hit ? "hit " : "miss", hit ? 0 : 1);
    }
    // Handle write access (write-through, no-write-allocate)
    else if (access_type == 'W' || access_type == 'w') {
        stats->mem_writes++;
        
        if (hit) {
            stats->hits++;
            update_lru(cache[index], config->associativity, hit_way);
        } else {
            stats->misses++;
            // No write allocate - don't load into cache
        }
        
        printf("%c %08x %x %x %x %s %d\n",
               access_type, address, tag, index, offset,
               hit ? "hit " : "miss", 1); // Always 1 mem ref for writes
    }
}

/**
 * Free cache memory
 */
void free_cache(CacheLine **cache, int num_sets) {
    for (int i = 0; i < num_sets; i++) {
        free(cache[i]);
    }
    free(cache);
}

/**
 * Validate cache configuration parameters
 */
int validate_config(CacheConfig *config) {
    // Check range limits
    if (config->num_sets <= 0 || config->num_sets > MAX_CACHE_SETS) {
        fprintf(stderr, "Error: Number of sets must be 1-%d\n", MAX_CACHE_SETS);
        return 0;
    }
    
    if (config->associativity <= 0 || config->associativity > MAX_ASSOCIATIVITY) {
        fprintf(stderr, "Error: Associativity must be 1-%d\n", MAX_ASSOCIATIVITY);
        return 0;
    }
    
    if (config->line_size < 8 || config->line_size > MAX_LINE_SIZE) {
        fprintf(stderr, "Error: Line size must be 8-%d bytes\n", MAX_LINE_SIZE);
        return 0;
    }
    
    // Check power of 2
    if ((config->num_sets & (config->num_sets - 1)) != 0) {
        fprintf(stderr, "Error: Number of sets must be a power of 2\n");
        return 0;
    }
    
    if ((config->line_size & (config->line_size - 1)) != 0) {
        fprintf(stderr, "Error: Line size must be a power of 2\n");
        return 0;
    }
    
    return 1;
}

/**
 * Main simulation function
 */
int main(void) {
    FILE *config_file = fopen("trace.config", "r");
    if (!config_file) {
        fprintf(stderr, "Error: Cannot open trace.config file\n");
        return 1;
    }
    
    // Read configuration
    CacheConfig config;
    if (fscanf(config_file, "Number of sets: %d\nSet size: %d\nLine size: %d",
               &config.num_sets, &config.associativity, &config.line_size) != 3) {
        fprintf(stderr, "Error: Invalid trace.config format\n");
        fclose(config_file);
        return 1;
    }
    fclose(config_file);
    
    // Validate configuration
    if (!validate_config(&config)) {
        return 1;
    }
    
    // Display configuration
    printf("Cache Simulator Configuration\n");
    printf("==============================\n");
    printf("Number of sets:    %d\n", config.num_sets);
    printf("Set associativity: %d\n", config.associativity);
    printf("Line size:         %d bytes\n", config.line_size);
    printf("Total cache size:  %d bytes\n", 
           config.num_sets * config.associativity * config.line_size);
    printf("\n");
    
    // Initialize cache
    CacheLine **cache;
    init_cache(&cache, &config);
    
    // Initialize statistics
    CacheStats stats = {0, 0, 0, 0};
    
    // Print header
    printf("Type Address  Tag      Index Offset Result MemRefs\n");
    printf("---- -------- -------- ----- ------ ------ -------\n");
    
    // Process trace
    char access_type;
    int size;
    unsigned int address;
    char line[256];
    
    while (fgets(line, sizeof(line), stdin)) {
        // Parse input line
        if (sscanf(line, "%c:%d:%x", &access_type, &size, &address) != 3) {
            continue; // Skip malformed lines
        }
        
        // Validate access size
        if (size != 1 && size != 2 && size != 4 && size != 8) {
            fprintf(stderr, "Warning: Invalid access size %d, skipping\n", size);
            continue;
        }
        
        // Check alignment
        if ((address & (size - 1)) != 0) {
            fprintf(stderr, "Warning: Misaligned access at 0x%x, skipping\n", address);
            continue;
        }
        
        // Simulate cache access
        access_cache(cache, &config, access_type, address, &stats);
    }
    
    // Print summary statistics
    int total_accesses = stats.hits + stats.misses;
    printf("\n");
    printf("Simulation Summary Statistics\n");
    printf("==============================\n");
    printf("Total accesses:    %d\n", total_accesses);
    printf("Hits:              %d\n", stats.hits);
    printf("Misses:            %d\n", stats.misses);
    printf("Hit rate:          %.2f%%\n", 
           total_accesses > 0 ? (100.0 * stats.hits / total_accesses) : 0.0);
    printf("Miss rate:         %.2f%%\n",
           total_accesses > 0 ? (100.0 * stats.misses / total_accesses) : 0.0);
    printf("Memory reads:      %d\n", stats.mem_reads);
    printf("Memory writes:     %d\n", stats.mem_writes);
    printf("Total memory refs: %d\n", stats.mem_reads + stats.mem_writes);
    
    // Cleanup
    free_cache(cache, config.num_sets);
    
    return 0;
}
