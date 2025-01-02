// utils.c
#include "utils.h"

CoinFlip flip_coin() {
    return (rand() % 2 == 0) ? TAILS : HEADS;
}

int flip_coins(int num) {
    int heads_count = 0;
    for (int i = 0; i < num; i++) {
        if (flip_coin() == HEADS) {
            heads_count++;
        }
    }
    return heads_count;
}
