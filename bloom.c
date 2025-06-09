#include "bloom.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BLOOM_SIZE 1024
#define MAX_HASHES 5

typedef unsigned int (*hash_func_t)(const char *str, int seed);

static unsigned char bit_array[BLOOM_SIZE];
static hash_func_t hash_funcs[MAX_HASHES];
static int num_hash_funcs = 0;

static unsigned int simple_hash(const char *str, int seed) {
    unsigned int hash = seed;
    while (*str) hash = hash * 101 + (unsigned char)(*str++);
    return hash % BLOOM_SIZE;
}

static unsigned int djb2_hash(const char *str, int seed) {
    unsigned int hash = 5381 + seed;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash % BLOOM_SIZE;
}

static unsigned int sdbm_hash(const char *str, int seed) {
    unsigned int hash = seed;
    int c;
    while ((c = *str++)) hash = c + (hash << 6) + (hash << 16) - hash;
    return hash % BLOOM_SIZE;
}

void bloom_init() {
    memset(bit_array, 0, sizeof(bit_array));
    num_hash_funcs = 0;
}

void bloom_free() {
    // Статическая память, ничего освобождать не нужно
}

int bloom_configure(const char *hash_list) {
    bloom_init();
    char buffer[256];
    strncpy(buffer, hash_list, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token = strtok(buffer, ",");
    while (token && num_hash_funcs < MAX_HASHES) {
        int id = atoi(token);
        switch (id) {
            case 1: hash_funcs[num_hash_funcs++] = simple_hash; break;
            case 2: hash_funcs[num_hash_funcs++] = djb2_hash; break;
            case 3: hash_funcs[num_hash_funcs++] = sdbm_hash; break;
            default: break;
        }
        token = strtok(NULL, ",");
    }

    return num_hash_funcs > 0;
}

void bloom_add(const char *key) {
    for (int i = 0; i < num_hash_funcs; i++) {
        unsigned int hash = hash_funcs[i](key, i + 17);
        bit_array[hash] = 1;
    }
}

int bloom_check(const char *key) {
    if (num_hash_funcs == 0) return 0; // Ничего не настроено → точно не в множестве
    for (int i = 0; i < num_hash_funcs; i++) {
        unsigned int hash = hash_funcs[i](key, i + 17);
        if (bit_array[hash] == 0) return 0;
    }
    return 1;
}
