//
//  hash_strings.h
//  HashFunctions
//
//  Created by Thomas Mailund on 20/11/2018.
//  Copyright © 2018 Thomas Mailund. All rights reserved.
//

#ifndef hash_strings_h
#define hash_strings_h

#include <stdint.h>

uint32_t additive_hash(uint32_t state, char *input, int len);
uint32_t rotating_hash(uint32_t state, char *input, int len);
uint32_t one_at_a_time_hash(uint32_t state, char *input, int len);
uint32_t jenkins_hash(uint32_t state, char *input, int len);

#endif /* hash_strings_h */
