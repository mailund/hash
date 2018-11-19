//
//  hash_set.c
//  ChainedHashSet
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
                        destructor_func destructor)
{
    while (list != 0) {
        struct linked_list *next = list->next;
        if (destructor && list->key)
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
                     compare_func cmp)
{
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


#pragma mark hash set

static void resize(struct hash_table *table, uint32_t new_size);
static void insert_key_hashed(struct hash_table *table, uint32_t hash_key, void *key);

static void resize(struct hash_table *table, uint32_t new_size)
{
    if (new_size == 0) return;

    // Remember these...
    uint32_t old_size = table->size;
    struct linked_list *old_bins = table->table;
    
    // Set up the new table
    table->table =
    (struct linked_list *)calloc(new_size, sizeof(struct linked_list));
    table->size = new_size;
    table->used = 0;
    
    // Copy keys
    for (int i = 0; i < old_size; ++i) {
        struct linked_list *list = &old_bins[i];
        while ( (list = list->next) ) {
            insert_key_hashed(table, list->hash_key, list->key);
        }
    }
    
    // Delete old table
    for (int i = 0; i < old_size; ++i) {
        delete_linked_list(old_bins[i].next, table->destructor);
    }
    free(old_bins);
}

struct hash_table *empty_table(uint32_t size,
                               hash_func hash,
                               compare_func cmp,
                               destructor_func destructor)
{
    struct hash_table *table = (struct hash_table *)malloc(sizeof(struct hash_table));
    // Using `calloc` here sets everything to zero. That initialises
    // the sentinel list links since it puts their next-pointers to null
    table->table = (struct linked_list *)calloc(size, sizeof(struct linked_list));
    table->size = size;
    table->used = 0;
    table->hash = hash;
    table->cmp = cmp;
    table->destructor = destructor;
    return table;
}

void delete_table(struct hash_table *table)
{
    for (int i = 0; i < table->size; ++i) {
        delete_linked_list(table->table[i].next, table->destructor);
    }
    free(table->table);
    free(table);
}

// Inserts when we already have the hash key. We have this to avoid
// hashing when we resize.
static void insert_key_hashed(struct hash_table *table, uint32_t hash_key, void *key)
{
    uint32_t mask = table->size - 1;
    uint32_t index = hash_key & mask;
    
    if (!list_contains_key(&table->table[index],
                           hash_key, key, table->cmp)) {
        list_insert_key(&table->table[index],
                        hash_key, key, table->cmp);
        table->used++;
    }
    
    if (table->used > table->size / 2)
        resize(table, table->size * 2);
}

void insert_key(struct hash_table *table, void *key)
{
    uint32_t hash_key = table->hash(key);
    insert_key_hashed(table, hash_key, key);
}

bool contains_key(struct hash_table *table, void *key)
{
    uint32_t hash_key = table->hash(key);
    uint32_t mask = table->size - 1;
    uint32_t index = hash_key & mask;
    return list_contains_key(&table->table[index],
                             hash_key, key,
                             table->cmp);
}

void delete_key(struct hash_table *table, void *key)
{
    uint32_t hash_key = table->hash(key);
    uint32_t mask = table->size - 1;
    uint32_t index = hash_key & mask;
    
    if (list_contains_key(&table->table[index], hash_key, key, table->cmp)) {
        list_delete_key(&table->table[index], hash_key, key, table->cmp, table->destructor);
        table->used--;
    }
    
    if (table->used < table->size / 8)
        resize(table, table->size / 2);
}

