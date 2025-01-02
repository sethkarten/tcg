#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

typedef enum {
    TAILS,
    HEADS
} CoinFlip;

CoinFlip flip_coin();
int flip_coins(int num);

#endif // UTILS_H
