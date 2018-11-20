//
//  hash.h
//  LinearProbeUniversalHashSet
//
//  Created by Thomas Mailund on 20/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//

#ifndef hash_h
#define hash_h

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t (*hash_func)(void *);
typedef void (*destructor_func)(void *);
typedef bool (*compare_func)(void *, void *);

struct hash_set {
    struct bin *table;
    uint32_t size;
    uint32_t used;
    uint32_t active;
    
    hash_func hash;
    compare_func cmp;
    destructor_func destructor;
    
    // For tabulation hashing.
    uint8_t *T, *T_end;
    
    float rehash_factor;
    unsigned int probe_limit;
    unsigned int operations_since_rehash;
};

struct hash_set *
empty_table        (uint32_t size, // Must be a power of two!
                    float rehash_factor,
                    hash_func hash,
                    compare_func cmp,
                    destructor_func destructor);
void delete_table  (struct hash_set *table);

void insert_key  (struct hash_set *table,
                  void *key);
bool contains_key(struct hash_set *table,
                  void *key);
void delete_key  (struct hash_set *table,
                  void *key);


#endif /* hash_h */
