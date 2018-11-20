//
//  main.c
//  Test
//
//  Created by Thomas Mailund on 20/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hash_map.h"


struct tag_key {
    bool key_deleted;
    bool val_deleted;
    uint32_t key;
};

static void init_tag_key(struct tag_key *tag_key, uint32_t key)
{
    tag_key->key = key;
    tag_key->val_deleted = tag_key->key_deleted = false;
}

static uint32_t random_key()
{
    return (uint32_t)random();
}

static bool compare_values(void *a, void *b)
{
    uint32_t key_a = ((struct tag_key*)a)->key;
    uint32_t key_b = ((struct tag_key*)b)->key;
    return key_a == key_b;
}

static uint32_t id_hash(void *key)
{
    return ((struct tag_key*)key)->key;
}

static void key_destroy(void *void_key)
{
    struct tag_key *key = (struct tag_key*)void_key;
    key->key_deleted = true;
}
static void val_destroy(void *void_key)
{
    struct tag_key *key = (struct tag_key*)void_key;
    key->val_deleted = true;
}

int main(int argc, const char *argv[])
{
    
    int no_elms = 100;
    struct tag_key keys[no_elms];
    for (int i = 0; i < no_elms; ++i) {
        init_tag_key(&keys[i], random_key());
    }
    struct tag_key other_keys[no_elms];
    for (int i = 0; i < no_elms; ++i) {
        init_tag_key(&other_keys[i], keys[i].key);
    }
    struct tag_key different_keys[no_elms];
    for (int i = 0; i < no_elms; ++i) {
        init_tag_key(&different_keys[i], random_key());
    }
    
    for (int i = 0; i < no_elms; ++i) {
        assert(keys[i].key_deleted == false);
        assert(keys[i].val_deleted == false);
        assert(other_keys[i].key_deleted == false);
        assert(other_keys[i].val_deleted == false);
    }
    
    struct hash_map *table = new_map(2, id_hash, compare_values, key_destroy, val_destroy);
    
    for (int i = 0; i < no_elms; ++i) {
        assert(keys[i].key_deleted == false);
        map(table, &keys[i], &keys[i]);
        assert(keys[i].key_deleted == false);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(contains_key(table, &keys[i]));
        assert(lookup(table, &keys[i]) == &keys[i]);
        assert(keys[i].key_deleted == false);
        assert(keys[i].val_deleted == false);
        assert(other_keys[i].key_deleted == false);
        assert(other_keys[i].val_deleted == false);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(contains_key(table, &other_keys[i]));
        assert(lookup(table, &other_keys[i]) == &keys[i]);
        assert(lookup(table, &other_keys[i]) != &other_keys[i]);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(!contains_key(table, &different_keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        map(table, &other_keys[i], &other_keys[i]);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(lookup(table, &other_keys[i]) != &keys[i]);
        assert(lookup(table, &other_keys[i]) == &other_keys[i]);
        assert(keys[i].key_deleted == true);
        assert(keys[i].val_deleted == true);
        assert(other_keys[i].key_deleted == false);
        assert(other_keys[i].val_deleted == false);
    }
    for (int i = 0; i < no_elms; ++i) {
        delete_key(table, &other_keys[i]);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(!contains_key(table, &keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(!contains_key(table, &other_keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(keys[i].key_deleted == true);
        assert(keys[i].val_deleted == true);
        assert(other_keys[i].key_deleted == true);
        assert(other_keys[i].val_deleted == true);
    }
    
    delete_map(table);
    printf("SUCCESS\n");
    
    return EXIT_SUCCESS;
}
