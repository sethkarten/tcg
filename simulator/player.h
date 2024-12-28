#ifndef PLAYER_H
#define PLAYER_H

#include "deck.h"

#define MAX_HAND_SIZE 20
#define MAX_BENCH_POKEMON 3
#define MAX_PRIZE_CARDS 3

typedef struct {
    Card *active_pokemon;
    Card bench[MAX_BENCH_POKEMON];
    int bench_count;
    Deck deck;
    Card hand[MAX_HAND_SIZE];
    int hand_count;
    Card prize_cards[MAX_PRIZE_CARDS];
    int prize_cards_left;
    Card discard_pile[MAX_CARDS_IN_DECK];
    int discard_count;
    Role role;
} Player;

typedef enum {
    PLAY, OPP
} Role;

void initialize_player(Player *player, Role role);

#endif // PLAYER_H
