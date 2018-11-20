//
//  hash_words.h
//  HashFunctions
//
//  Created by Thomas Mailund on 20/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//

#ifndef hash_words_h
#define hash_words_h

#include <stdint.h>

uint32_t additive_hash(uint32_t state, uint32_t input);
uint32_t rotating_hash(uint32_t state, uint32_t input);
uint32_t rotating_hash_rev(uint32_t state, uint32_t input);
uint32_t one_at_a_time_hash(uint32_t state, uint32_t input);
uint32_t one_at_a_time_hash_rev(uint32_t state, uint32_t input);
uint32_t jenkins_hash(uint32_t state, uint32_t input);

#endif /* hash_words_h */
