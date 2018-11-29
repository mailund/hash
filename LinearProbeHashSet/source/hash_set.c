//
//  hash_set.c
//  LinearProbeHashSet
//
//  Created by Thomas Mailund on 19/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//

#include <stdlib.h>
#include "hash_set.h"

struct bin {
    bool is_free : 1;
    bool is_deleted : 1;
    uint32_t hash_key;
    void *key;
};

static uint32_t
p(uint32_t k, unsigned int i, unsigned int m)
{
    return (k + i) & (m - 1);
}

void insert_key_hashed(struct hash_set *table,
                       uint32_t hash_key, void *key);
static bool contains_key_hashed(struct hash_set *table, uint32_t hash_key, void *key);

static void resize(struct hash_set *table, uint32_t new_size)
{
    if (new_size == 0) return;

    // Remember the old bins until we have moved them.
    struct bin *old_bins = table->table;
    uint32_t old_size = table->size;
    
    // Update table so it now contains the new bins
    table->table =
    (struct bin *)malloc(new_size * sizeof(struct bin));
    struct bin *end = table->table + new_size;
    for (struct bin *bin = table->table; bin != end; ++bin) {
        bin->is_free = true;
        bin->is_deleted = false;
    }
    table->size = new_size;
    table->active = table->used = 0;
    
    // Move the values from the old bins to the new,
    // using the table's insertion function
    end = old_bins + old_size;
    for (struct bin *bin = old_bins; bin != end; ++bin) {
        if (bin->is_free || bin->is_deleted) continue;
        insert_key_hashed(table, bin->hash_key, bin->key);
    }
    
    // Finally, free memory for old bins
    free(old_bins);
}

struct hash_set *new_set(uint32_t size,
                               hash_func  hash,
                               compare_func cmp,
                               destructor_func destructor)
{
    struct hash_set *table =
    (struct hash_set*)malloc(sizeof(struct hash_set));
    table->table =
    (struct bin *)malloc(size * sizeof(struct bin));
    struct bin *end = table->table + size;
    for (struct bin *bin = table->table; bin != end; ++bin) {
        bin->is_free = true;
        bin->is_deleted = false;
    }
    table->size = size;
    table->active = table->used = 0;
    table->hash = hash;
    table->cmp = cmp;
    table->destructor = destructor;
    return table;
}

void delete_set(struct hash_set *table)
{
    if (table->destructor) {
        struct bin *end = table->table + table->size;
        for (struct bin *bin = table->table; bin != end; ++bin) {
            if (bin->is_free || bin->is_deleted) continue;
            table->destructor(bin->key);
        }
    }
    free(table->table);
    free(table);
}

void insert_key_hashed(struct hash_set *table,
                       uint32_t hash_key, void *key)
{
    uint32_t index;
    bool contains = contains_key_hashed(table, hash_key, key);
    for (uint32_t i = 0; i < table->size; ++i) {
        index = p(hash_key, i, table->size);
        struct bin *bin = & table->table[index];
        
        if (bin->is_free) {
            bin->hash_key = hash_key; bin->key = key;
            bin->is_free = bin->is_deleted = false;
            
            // we have one more active element
            // and one more unused cell changes character
            table->active++; table->used++;
            break;
        }
        
        if (bin->is_deleted && !contains) {
            bin->hash_key = hash_key; bin->key = key;
            bin->is_free = bin->is_deleted = false;
            
            // we have one more active element
            // but we do not use more cells since the
            // deleted cell was already used.
            table->active++;
            break;
        }
        
        if (bin->hash_key == hash_key) {
            if (table->cmp(bin->key, key)) {
                table->destructor(bin->key);
                bin->key = key;
                return; // Done
            } else {
                // we have found the key but with as
                // different value...
                // Continue searching
                continue;
            }
        }
    }
    
    if (table->used > table->size / 2)
        resize(table, table->size * 2);
}

void insert_key(struct hash_set *table, void *key)
{
    uint32_t hash_key = table->hash(key);
    insert_key_hashed(table, hash_key, key);
}

static bool contains_key_hashed(struct hash_set *table, uint32_t hash_key, void *key)
{
    for (uint32_t i = 0; i < table->size; ++i) {
        uint32_t index = p(hash_key, i, table->size);
        struct bin *bin = & table->table[index];
        if (bin->is_free)
            return false;
        if (!bin->is_deleted && bin->hash_key == hash_key &&
            table->cmp(bin->key, key))
            return true;
    }
    return false;
}

bool contains_key(struct hash_set *table, void *key)
{
    uint32_t hash_key = table->hash(key);
    return contains_key_hashed(table, hash_key, key);
}

void delete_key(struct hash_set *table, void *key)
{
    uint32_t hash_key = table->hash(key);
    for (uint32_t i = 0; i < table->size; ++i) {
        uint32_t index = p(hash_key, i, table->size);
        struct bin * bin = & table->table[index];
        
        if (bin->is_free) return;
        
        if (!bin->is_deleted && bin->hash_key == hash_key &&
            table->cmp(bin->key, key)) {
            bin->is_deleted = true;
            if (table->destructor)
                table->destructor(table->table[index].key);
            table->active--;
            break;
        }
    }
    
    if (table->active < table->size / 8)
        resize(table, table->size / 2);
}
