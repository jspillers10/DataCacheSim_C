#!/bin/bash
# Comprehensive Test Suite for Cache Simulator
# Author: Jake Spillers

echo "========================================="
echo "Cache Simulator Test Suite"
echo "========================================="
echo ""

# Test 1: Basic Hit/Miss Pattern
echo "Test 1: Basic Hit/Miss Pattern"
echo "==============================="
cat > test1_trace.txt << EOF
R:4:00000000
R:4:00000004
R:4:00000008
R:4:0000000c
R:4:00000000
R:4:00000004
EOF

cat > trace.config << EOF
Number of sets: 4
Set size: 1
Line size: 16
EOF

./cache_simulator < test1_trace.txt > test1_output.txt
echo "Expected: 2 hits, 4 misses (33.33% hit rate)"
grep "Hit rate" test1_output.txt
echo ""

# Test 2: LRU Replacement
echo "Test 2: LRU Replacement"
echo "======================="
cat > test2_trace.txt << EOF
R:4:00000000
R:4:00000010
R:4:00000020
R:4:00000000
R:4:00000030
R:4:00000010
EOF

cat > trace.config << EOF
Number of sets: 1
Set size: 2
Line size: 16
EOF

./cache_simulator < test2_trace.txt > test2_output.txt
echo "Expected: LRU should evict 0x00000020 when 0x00000030 is loaded"
echo "Then 0x00000010 should hit (was recently used)"
grep "Result" test2_output.txt | tail -2
echo ""

# Test 3: Write-Through Policy
echo "Test 3: Write-Through Policy"
echo "============================="
cat > test3_trace.txt << EOF
R:4:00000000
W:4:00000000
W:4:00000004
W:4:00000100
R:4:00000100
EOF

cat > trace.config << EOF
Number of sets: 4
Set size: 2
Line size: 16
EOF

./cache_simulator < test3_trace.txt > test3_output.txt
echo "Expected: All writes go to memory"
grep "Memory writes" test3_output.txt
echo ""

# Test 4: Conflict Misses
echo "Test 4: Conflict Misses"
echo "======================="
cat > test4_trace.txt << EOF
R:4:00000000
R:4:00000100
R:4:00000200
R:4:00000000
R:4:00000100
EOF

cat > trace.config << EOF
Number of sets: 16
Set size: 1
Line size: 16
EOF

./cache_simulator < test4_trace.txt > test4_output.txt
echo "Expected: Conflict misses when addresses map to same set"
grep "Misses:" test4_output.txt
echo ""

# Test 5: Spatial Locality
echo "Test 5: Spatial Locality"
echo "========================"
cat > test5_trace.txt << EOF
R:4:00000000
R:4:00000004
R:4:00000008
R:4:0000000c
EOF

cat > trace.config << EOF
Number of sets: 8
Set size: 2
Line size: 16
EOF

./cache_simulator < test5_trace.txt > test5_output.txt
echo "Expected: All hits after first miss (same cache line)"
grep "Hit rate" test5_output.txt
echo ""

echo "========================================="
echo "All tests completed!"
echo "========================================="
echo ""
echo "Output files created:"
echo "  test1_output.txt - Basic hit/miss"
echo "  test2_output.txt - LRU verification"
echo "  test3_output.txt - Write policy"
echo "  test4_output.txt - Conflict misses"
echo "  test5_output.txt - Spatial locality"
