# Makefile for Cache Simulator
# Author: Jake Spillers

# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -g
TARGET = cache_simulator
SOURCE = cache_simulator.c

# Default target
all: $(TARGET)

# Build the simulator
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Run with sample trace
test: $(TARGET)
	@echo "Running cache simulator with sample trace..."
	@./$(TARGET) < sample_trace.txt

# Run with custom trace
run: $(TARGET)
	@echo "Reading trace from stdin..."
	@./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f core
	rm -f *~

# Create sample configuration
config:
	@echo "Number of sets: 16" > trace.config
	@echo "Set size: 2" >> trace.config
	@echo "Line size: 16" >> trace.config
	@echo "Created trace.config with default parameters"

# Show help
help:
	@echo "Cache Simulator Build System"
	@echo "============================"
	@echo "Targets:"
	@echo "  make          - Build the cache simulator"
	@echo "  make test     - Run with sample trace"
	@echo "  make run      - Run with stdin input"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make config   - Create default trace.config"
	@echo "  make help     - Show this help message"

.PHONY: all test run clean config help
