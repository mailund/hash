//
//  hash_set.c
//  ChainedUniversalHashSet
//
//  Created by Thomas Mailund on 19/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//

#include "hash_set.h"
#include <stdlib.h>

#pragma mark linked lists
struct linked_list {
    uint32_t hash_key;
    void *key;
    struct linked_list *next;
};

void delete_linked_list(struct linked_list *list,
                        destructor_func destructor,
                        bool delete_keys)
{
    while (list != 0) {
        struct linked_list *next = list->next;
        if (delete_keys && destructor && list->key)
            destructor(list->key);
        free(list);
        list = next;
    }
}

static struct linked_list *
get_previous_link(struct linked_list *list,
                  uint32_t hash_key, void *key,
                  compare_func cmp)
{
    while (list->next) {
        if (list->next->hash_key == hash_key &&
            cmp(list->next->key, key))
            return list;
        list = list->next;
    }
    return 0;
}

void list_insert_key(struct linked_list *list,
                     uint32_t hash_key,
                     void *key,
                     compare_func cmp,
                     destructor_func destructor)
{
    struct linked_list *link = get_previous_link(list, hash_key, key, cmp);
    if (link) {
        link = link->next;
        destructor(link->key);
        link->key = key;
        return;
    }
    
    // build link and put it at the front of the list.
    // the hash table checks for duplicates if we want to avoid those
    struct linked_list *new_link = (struct linked_list*)malloc(sizeof(struct linked_list));
    new_link->hash_key = hash_key;
    new_link->key = key;
    new_link->next = list->next;
    list->next = new_link;
}

void list_delete_key(struct linked_list *list,
                     uint32_t hash_key,
                     void *key,
                     compare_func cmp,
                     destructor_func destructor)
{
    struct linked_list *link = get_previous_link(list, hash_key, key, cmp);
    if (!link) return;
    
    struct linked_list *to_delete = link->next;
    link->next = to_delete->next;
    if (destructor) destructor(to_delete->key);
    free(to_delete);
}

bool list_contains_key(struct linked_list *list,
                       uint32_t hash_key,
                       void *key,
                       compare_func cmp)
{
    return get_previous_link(list, hash_key, key, cmp) != 0;
}

#pragma mark universal hashing

void tabulation_sample(uint32_t *start, uint32_t *end)
{
    while (start != end)
        *(start++) = rand();
}

// tabulation hashing, r=4, q=32
static uint32_t tabhash(uint32_t x, uint8_t *T)
{
    const int r = 4;
    const uint32_t no_cols = 1 << r;
    const uint32_t mask = (1 << r) - 1;
    
    uint32_t *T_ = (uint32_t*)T;
    uint32_t y = T_[0 * no_cols + (x & mask)]; x >>= r;
    y ^= T_[1 * no_cols + (x & mask)]; x >>= r;
    y ^= T_[2 * no_cols + (x & mask)]; x >>= r;
    y ^= T_[3 * no_cols + (x & mask)]; x >>= r;
    y ^= T_[4 * no_cols + (x & mask)]; x >>= r;
    y ^= T_[5 * no_cols + (x & mask)]; x >>= r;
    y ^= T_[6 * no_cols + (x & mask)]; x >>= r;
    y ^= T_[7 * no_cols + (x & mask)];
    
    return y;
}


#pragma mark hash set

static void resize(struct hash_set *table, uint32_t new_size);
static void insert_key_hashed(struct hash_set *table,
                              uint32_t hash_key,
                              uint32_t uhash_key,
                              void *key);

static void resize(struct hash_set *table, uint32_t new_size)
{
    if (new_size == 0) return;
    
    // Remember these...
    uint32_t old_size = table->size;
    struct linked_list *old_bins = table->table;
    
    // Set up the new table
    table->table = (struct linked_list *)calloc(new_size, sizeof(struct linked_list));
    table->size = new_size;
    table->used = 0;
    
    // Update hash function
    tabulation_sample((uint32_t*)table->T, (uint32_t*)table->T_end);
    
    // Update rehash limit
    table->probe_limit = table->rehash_factor * new_size;
    table->operations_since_rehash = 0;

    // Copy keys
    for (int i = 0; i < old_size; ++i) {
        struct linked_list *list = &old_bins[i];
        while ( (list = list->next) ) {
            uint32_t new_uhash_key = tabhash(list->hash_key, table->T);
            insert_key_hashed(table, list->hash_key, new_uhash_key, list->key);
        }
    }
    
    // Delete old table
    for (int i = 0; i < old_size; ++i) {
        delete_linked_list(old_bins[i].next, table->destructor, false);
    }
    

    free(old_bins);
}

static void rehash(struct hash_set *table)
{
    // Remember these...
    uint32_t old_size = table->size;
    struct linked_list *old_bins = table->table;
    
    // Set up the new table
    table->table = (struct linked_list *)calloc(table->size, sizeof(struct linked_list));
    table->used = 0;
    
    // Update hash function
    tabulation_sample((uint32_t*)table->T, (uint32_t*)table->T_end);
    
    // Update rehash limit
    table->operations_since_rehash = 0;
    
    // Copy keys
    for (int i = 0; i < old_size; ++i) {
        struct linked_list *list = &old_bins[i];
        while ( (list = list->next) ) {
            uint32_t new_uhash_key = tabhash(list->hash_key, table->T);
            insert_key_hashed(table, list->hash_key, new_uhash_key, list->key);
        }
    }
    
    // Delete old table
    for (int i = 0; i < old_size; ++i) {
        delete_linked_list(old_bins[i].next, table->destructor, false);
    }
    
    free(old_bins);
}

struct hash_set *new_set(uint32_t size,
                               float rehash_factor,
                               hash_func hash,
                               compare_func cmp,
                               destructor_func destructor)
{
    struct hash_set *table = (struct hash_set *)malloc(sizeof(struct hash_set));
    
    // Using `calloc` here sets everything to zero. That initialises
    // the sentinel list links since it puts their next-pointers to null
    table->table = (struct linked_list *)calloc(size, sizeof(struct linked_list));
    
    table->size = size;
    table->used = 0;
    table->hash = hash;
    table->cmp = cmp;
    table->destructor = destructor;
    
    // setting up tabulation hashing table
    int p = 32;
    int r = 4;
    int q = 32;
    int no_cols = (1 << r);
    int t = p / r;
    int bytes = t * no_cols * q / 8;
    table->T = malloc(bytes);
    table->T_end = table->T + bytes;
    tabulation_sample((uint32_t*)table->T,
                      (uint32_t*)table->T_end);
    
    table->rehash_factor = rehash_factor;
    table->probe_limit = rehash_factor * size;
    table->operations_since_rehash = 0;

    return table;
}

void delete_set(struct hash_set *table)
{
    for (int i = 0; i < table->size; ++i) {
        delete_linked_list(table->table[i].next, table->destructor, true);
    }
    free(table->table);
    free(table->T);
    free(table);
}

// Inserts when we already have the hash key. We have this to avoid
// hashing when we resize. This function does not trigger rehashing
// or resizing
static void insert_key_hashed(struct hash_set *table, uint32_t hash_key, uint32_t uhash_key, void *key)
{
    uint32_t mask = table->size - 1;
    uint32_t index = uhash_key & mask;
    
    if (!list_contains_key(&table->table[index],
                           hash_key, key, table->cmp)) {
        table->used++;
        
    }
    list_insert_key(&table->table[index],
                    hash_key, key, table->cmp,
                    table->destructor);
    
    if (table->used > table->size / 2)
        resize(table, table->size * 2);
}

void insert_key(struct hash_set *table, void *key)
{
    table->operations_since_rehash++;
    if (table->operations_since_rehash > table->probe_limit) {
        rehash(table);
    }

    uint32_t hash_key = table->hash(key);
    uint32_t uhash_key = tabhash(hash_key, table->T);
    insert_key_hashed(table, hash_key, uhash_key, key);
    if (table->used > table->size / 2)
        resize(table, table->size * 2);
}

bool contains_key(struct hash_set *table, void *key)
{
    table->operations_since_rehash++;
    if (table->operations_since_rehash > table->probe_limit) {
        rehash(table);
    }

    uint32_t hash_key = table->hash(key);
    uint32_t uhash_key = tabhash(hash_key, table->T);
    uint32_t mask = table->size - 1;
    uint32_t index = uhash_key & mask;
    return list_contains_key(&table->table[index],
                             hash_key, key,
                             table->cmp);
}

void delete_key(struct hash_set *table, void *key)
{
    table->operations_since_rehash++;
    if (table->operations_since_rehash > table->probe_limit) {
        rehash(table);
    }

    uint32_t hash_key = table->hash(key);
    uint32_t uhash_key = tabhash(hash_key, table->T);
    uint32_t mask = table->size - 1;
    uint32_t index = uhash_key & mask;
    
    if (list_contains_key(&table->table[index], hash_key, key, table->cmp)) {
        list_delete_key(&table->table[index], hash_key, key, table->cmp, table->destructor);
        table->used--;
    }
    
    if (table->used < table->size / 8)
        resize(table, table->size / 2);
}

