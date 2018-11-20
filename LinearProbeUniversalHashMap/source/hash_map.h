//
//  hash_map.h
//  LinearProbeUniversalHashMap
//
//  Created by Thomas Mailund on 20/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//

#ifndef hash_map_h
#define hash_map_h

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t (*hash_func)(void *);
typedef void (*destructor_func)(void *);
typedef bool (*compare_func)(void *, void *);

struct hash_map {
    struct bin *table;
    uint32_t size;
    uint32_t used;
    uint32_t active;
    
    hash_func hash;
    compare_func key_cmp;
    destructor_func key_destructor;
    destructor_func val_destructor;
    
    // For tabulation hashing.
    uint8_t *T, *T_end;
    
    float rehash_factor;
    unsigned int probe_limit;
    unsigned int operations_since_rehash;
};

struct hash_map *
new_map           (uint32_t size, // Must be a power of two!
                   float rehash_factor,
                   hash_func hash,
                   compare_func key_cmp,
                   destructor_func key_destructor,
                   destructor_func val_destructor);
void  delete_map  (struct hash_map *table);

void  map          (struct hash_map *table,
                    void *key, void *val);
void *lookup       (struct hash_map *table, void *key);
bool  contains_key (struct hash_map *table, void *key);
void  delete_key   (struct hash_map *table, void *key);

#endif /* hash_map_h */
