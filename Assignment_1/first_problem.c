#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86intrin.h>

#define NUM_TRIALS 100000
#define ARRAY_SIZE (1024 * 1024)
#define SAMPLE_SAVE_COUNT 5

static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ (
        "lfence\n\t"
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi)
        :
        : "memory"
    );
    return ((uint64_t)hi << 32) | lo;
}

static inline void memory_access(volatile int* addr) {
    *addr;
}

int main() {
    int* array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    if (!array) {
        perror("Memory allocation failed");
        return 1;
    }

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (int)i;
    }

    volatile int* addr = &array[ARRAY_SIZE / 2];
    uint64_t total_cache_cycles = 0, total_dram_cycles = 0;
    uint64_t cache_samples[SAMPLE_SAVE_COUNT];
    uint64_t dram_samples[SAMPLE_SAVE_COUNT];

    // Measure cache access
    for (int i = 0; i < NUM_TRIALS; i++) {
        memory_access(addr);
        uint64_t start = rdtsc();
        memory_access(addr);
        uint64_t end = rdtsc();
        uint64_t delta = end - start;

        if (i < SAMPLE_SAVE_COUNT) {
            cache_samples[i] = delta;
        }

        total_cache_cycles += delta;
    }

    // Measure DRAM access
    for (int i = 0; i < NUM_TRIALS; i++) {
        _mm_clflush((void*)addr);
        usleep(1); // 1 ms
        uint64_t start = rdtsc();
        memory_access(addr);
        uint64_t end = rdtsc();
        uint64_t delta = end - start;

        if (i < SAMPLE_SAVE_COUNT) {
            dram_samples[i] = delta;
        }

        total_dram_cycles += delta;
    }

    double avg_cache = (double)total_cache_cycles / NUM_TRIALS;
    double avg_dram = (double)total_dram_cycles / NUM_TRIALS;

    printf("Memory Type\tMeasured Latency (cycles)\n");
    printf("Cache      \t%.2f\n", avg_cache);
    printf("DRAM       \t%.2f\n", avg_dram);

    // Write samples to file
    FILE *fp = fopen("latencies.txt", "w");
    if (!fp) {
        perror("Could not open latencies.txt for writing");
        free(array);
        return 1;
    }

    fprintf(fp, "Sample#\tCache\tDRAM\n");
    for (int i = 0; i < SAMPLE_SAVE_COUNT; i++) {
        fprintf(fp, "%d\t%lu\t%lu\n", i + 1,
                (unsigned long)cache_samples[i],
                (unsigned long)dram_samples[i]);
    }

    fclose(fp);
    free(array);
    return 0;
}
