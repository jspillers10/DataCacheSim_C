# Data Cache Simulator

A high-performance cache simulator implementing set-associative caching with LRU replacement policy, write-through, and no-write-allocate policies.

## Project Overview

This project simulates a configurable data cache system to analyze cache performance characteristics. The simulator models real-world cache behavior to evaluate hit/miss rates, memory access patterns, and overall system performance under various workloads.

### Key Features

- Configurable cache parameters (sets, associativity, line size)
- LRU (Least Recently Used) replacement policy
- Write-through cache policy
- No-write-allocate on write misses
- Comprehensive statistics tracking
- Input validation and error handling
- Clean, well-documented code

## Technical Specifications

### Cache Policies

**Replacement Policy**: LRU (Least Recently Used)
- Tracks access recency for each cache line
- Evicts the least recently used line on capacity misses

**Write Policy**: Write-Through
- All writes immediately update main memory
- Cache is updated on write hits

**Write Miss Policy**: No-Write-Allocate
- Write misses don't load data into cache
- Reduces cache pollution from write-only data

### Configuration Constraints

| Parameter | Minimum | Maximum | Constraint |
|-----------|---------|---------|------------|
| Sets | 1 | 8,192 | Power of 2 |
| Associativity | 1 | 8 | Any positive integer |
| Line Size | 8 bytes | 64 bytes | Power of 2 |

## Compilation & Usage

### Compilation

```bash
gcc -std=c99 -Wall -Wextra -O2 -o cache_simulator cache_simulator.c
```

### Configuration File

Create a `trace.config` file:

```
Number of sets: 16
Set size: 2
Line size: 16
```

### Trace File Format

Input traces use the format: `AccessType:Size:Address`

- **AccessType**: `R` (read) or `W` (write)
- **Size**: Access size in bytes (1, 2, 4, or 8)
- **Address**: Hexadecimal memory address

Example trace:
```
R:4:00000000
W:4:0000000c
R:4:00000100
```

### Running the Simulator

```bash
./cache_simulator < trace_file.txt
```

## Output Format

### Per-Access Output

```
Type Address  Tag      Index Offset Result MemRefs
---- -------- -------- ----- ------ ------ -------
R    00000000 0        0     0      miss   1
R    00000004 0        0     4      hit    0
W    0000000c 0        0     c      hit    1
```

### Summary Statistics

```
Simulation Summary Statistics
==============================
Total accesses:    15
Hits:              5
Misses:            10
Hit rate:          33.33%
Miss rate:         66.67%
Memory reads:      10
Memory writes:     2
Total memory refs: 12
```

## Architecture

### Data Structures

**CacheLine**
- `valid`: Valid bit indicating if line contains data
- `tag`: Tag bits for address matching
- `lru_counter`: Counter for LRU replacement tracking

**CacheConfig**
- `num_sets`: Number of cache sets
- `associativity`: Lines per set
- `line_size`: Bytes per line
- `offset_bits`: Calculated offset field size
- `index_bits`: Calculated index field size

### Core Algorithms

**Address Decomposition**
```
Address = | Tag | Index | Offset |
```

**LRU Tracking**
- Each access increments accessed line's counter to max
- Decrements counters of more recent lines
- Victim selection finds minimum counter value

## Implementation Highlights

### Correct Bit Manipulation

The original code incorrectly used cache configuration values directly instead of computing the required bit counts. The fixed implementation properly calculates offset and index bit widths:

```c
unsigned int offset = address & ((1 << offset_bits) - 1);
unsigned int index = (address >> offset_bits) & ((1 << index_bits) - 1);
unsigned int tag = address >> (offset_bits + index_bits);
```

This ensures accurate address decomposition regardless of cache configuration.

### Complete LRU Implementation

LRU replacement requires tracking access recency for each cache line. The implementation uses a counter-based approach where each access updates the counters:

```c
void update_lru(CacheLine *set, int associativity, int accessed_way) {
    unsigned int accessed_counter = set[accessed_way].lru_counter;
    
    for (int i = 0; i < associativity; i++) {
        if (set[i].valid && set[i].lru_counter > accessed_counter) {
            set[i].lru_counter--;
        }
    }
    
    set[accessed_way].lru_counter = associativity - 1;
}
```

This approach efficiently maintains LRU ordering without requiring timestamps or additional data structures.

### Memory Reference Counting

The simulator properly distinguishes between different types of memory accesses:

- Read misses generate one memory read
- Write hits and misses both generate one memory write (write-through policy)
- Write misses do not load data into the cache (no-write-allocate policy)

Statistics are tracked separately for reads and writes to provide detailed performance analysis.

## Testing

### Test Case 1: Basic Functionality
- Configuration: 16 sets, 2-way, 16-byte lines
- Tests: Cold misses, capacity misses, write-through behavior

### Test Case 2: LRU Verification
- Tests LRU replacement by accessing multiple lines per set
- Verifies correct victim selection

### Test Case 3: Write Policy
- Tests write-through and no-write-allocate
- Verifies memory reference counts

## Performance Analysis

### Cache Performance Metrics

**Hit Rate** = Hits / (Hits + Misses)
- Percentage of accesses served from cache
- Higher is better (less memory latency)

**Miss Rate** = Misses / (Hits + Misses)
- Percentage of accesses requiring memory
- Lower is better

**Average Memory Access Time (AMAT)**
```
AMAT = Hit_Time + (Miss_Rate × Miss_Penalty)
```

## Example Analysis

Running the simulator with the sample trace on a 16-set, 2-way associative cache with 16-byte lines produces the following results:

- Hit Rate: 33.33% (5 hits out of 15 total accesses)
- Miss Rate: 66.67% (10 misses out of 15 total accesses)
- Total Memory References: 12 (10 reads from misses + 2 writes)

The relatively low hit rate occurs because most accesses in this trace are compulsory misses - the first access to each cache line. The working set size also exceeds the cache capacity given the access pattern. Increasing either the cache size or associativity would reduce conflict misses and improve the hit rate for this workload.

## Improvements from Original Implementation

The original implementation contained several critical issues that prevented proper functionality. The following improvements were made:

1. Corrected bit manipulation for proper tag/index/offset extraction
2. Implemented complete LRU replacement policy with counter-based tracking
3. Fixed memory reference counting to accurately track reads and writes
4. Improved input validation with comprehensive parameter checking
5. Restructured code into modular, well-documented functions
6. Corrected output formatting and removed debug statements
7. Added memory safety checks for allocation verification and proper cleanup

These changes transformed the simulator from a non-functional prototype into a production-ready tool suitable for cache performance analysis.

## Learning Outcomes

This project demonstrates practical understanding of:

- Cache architecture and organization principles
- Memory hierarchy design and optimization
- Replacement policies and their implementations
- Write policies including write-through and no-write-allocate
- Bit manipulation and address decomposition techniques
- Performance analysis and system optimization
- Systems programming in C with focus on efficiency and correctness

## Author

Jake Spillers  
Cyber Security Specialist

## License

Academic project - Educational use permitted with attribution.

---

Built for CDA3100 - Computer Organization  
Refined for professional portfolio presentation
