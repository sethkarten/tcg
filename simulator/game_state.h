#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "player.h"
#include "card.h"
#include "hash.h"

#define MAX_CARDS 512

typedef struct {
    bool blaine_boost;
    bool giovanni_boost;
    bool sabrina_switch;
    bool blue_protection;
    int retreat_reduction;
} TurnEffects;

typedef struct {
    Player player1;
    Player player2;
    int current_turn;
    bool supporter_played;
    HashMap* card_dictionary;
    bool game_over;
    TurnEffects turn_effects;
} GameState;

void initialize_game_state(GameState *game, const char deck1[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH], 
                           const char deck2[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH]);
void act_turn(GameState *game, Player *player, char ** action, int num_actions);

#endif // GAME_STATE_H
