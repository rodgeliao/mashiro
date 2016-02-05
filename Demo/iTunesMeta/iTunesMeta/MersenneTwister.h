//
//  MersenneTwister.h
//  MersenneTwister
//
//  Created by BlueCocoa on 16/2/2.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#ifndef MersenneTwister_H
#define MersenneTwister_H

#include <stdint.h>
#include <sys/types.h>

class MersenneTwister {
private:
    uint32_t state[624];
    size_t index;
    
    void reseed();
public:
    MersenneTwister(uint32_t seed);
    uint32_t rand();
};

#endif /* MersenneTwister_H */
