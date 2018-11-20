//
//  main.c
//  Test
//
//  Created by Thomas Mailund on 19/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hash_set.h"


struct tag_key {
    bool deleted;
    uint32_t key;
};

static void init_tag_key(struct tag_key *tag_key, uint32_t key)
{
    tag_key->key = key;
    tag_key->deleted = false;
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

static void destroy(void *void_key)
{
    struct tag_key *key = (struct tag_key*)void_key;
    key->deleted = true;
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
    
    struct hash_set *table =
        new_set(2, id_hash, compare_values, destroy);
    for (int i = 0; i < no_elms; ++i) {
        insert_key(table, &keys[i]);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(keys[i].deleted == false);
        assert(other_keys[i].deleted == false);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(contains_key(table, &keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(contains_key(table, &other_keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(!contains_key(table, &different_keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        insert_key(table, &other_keys[i]);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(keys[i].deleted == true);
        assert(other_keys[i].deleted == false);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(contains_key(table, &keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(contains_key(table, &other_keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        delete_key(table, &other_keys[i]);
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(!contains_key(table, &other_keys[i]));
    }
    for (int i = 0; i < no_elms; ++i) {
        assert(keys[i].deleted == true);
        assert(other_keys[i].deleted == true);
    }
    
    delete_set(table);
    printf("SUCCESS\n");
    
    return EXIT_SUCCESS;
}
