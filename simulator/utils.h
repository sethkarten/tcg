#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef enum { TAILS, HEADS } CoinFlip;

CoinFlip flip_coin() {
    return (rand() % 2 == 0) ? TAILS : HEADS;
}

// Function to flip N coins and return the number of heads
int flip_coins(int num) {
    int heads_count = 0;
    for (int i = 0; i < num; i++) {
        if (flip_coin() == HEADS) {
            heads_count++;
        }
    }
    return heads_count;
}

#endif // UTILS_H
