#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "player.h"
#include "card.h"
#include "hash.h"

#define MAX_CARDS 512

typedef struct {
    Player player1;
    Player player2;
    int current_turn;
    HashMap* card_dictionary;
    bool game_over;
} GameState;

void initialize_game_state(GameState *game, const char deck1[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH], 
                           const char deck2[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH]);
void act_turn(GameState *game, Player *player, char *** actions, int num_actions);

#endif // GAME_STATE_H
