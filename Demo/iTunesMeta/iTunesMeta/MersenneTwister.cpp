//
//  MersenneTwister.cpp
//  MersenneTwister
//
//  Created by BlueCocoa on 16/2/2.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#include "MersenneTwister.h"

MersenneTwister::MersenneTwister(uint32_t seed) {
    this->index = 0;
    this->state[0] = seed;
    for (size_t i = 1; i < 624; i++)
        this->state[i] = (i + 0x6C078965 * (this->state[i - 1] ^ (this->state[i - 1] >> 30))) & 0xFFFFFFFF;
}

uint32_t MersenneTwister::rand() {
    if (this->index == 0)
        this->reseed();
    
    uint32_t y = this->state[this->index];
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680;
    y ^= (y << 15) & 0xEFC60000;
    y ^= (y >> 18);
    
    this->index = (this->index + 1) % 624;
    
    return y;
}

void MersenneTwister::reseed() {
    for (size_t i = 0; i < 624; i++) {
        uint32_t y = (this->state[i] & 0x80000000) + (this->state[(i + 1) % 624] & 0x7FFFFFFF);
        this->state[i] = this->state[(i + 397) % 624] ^ (y >> 1);
        if (y % 2 != 0)
            this->state[i] ^= 0x9908B0DF;
    }
}
