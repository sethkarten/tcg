#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <time.h>

// Function to flip N coins and return the number of heads
int flip_coins(int N) {
    int heads = 0;
    
    for (int i = 0; i < N; i++) {
        if (rand() % 2 == 1) {
            heads++;
        }
    }
    
    return heads;
}

#endif // UTILS_H
