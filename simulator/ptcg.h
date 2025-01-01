#ifndef PTCG_H
#define PTCG_H

#include "game_state.h"

#define MAX_PRIZE_CARDS 3
#define MAX_BENCH_POKEMON 3
#define MAX_ENERGY_TYPES 4
#define MAX_HAND_SIZE 20

// Function prototypes
void reset_game(GameState *game, 
                const char player1_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH], 
                bool player1_energy[MAX_CARD_ENERGIES], 
                const char player2_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                 bool player2_energy[MAX_CARD_ENERGIES]);
bool * get_legal_actions(GameState *game, int *actions);
int execute_action(GameState *game, int action, int target, int opponent_target);
void get_observation(GameState *game, float *observation);
bool is_game_over(GameState *game);

#endif // PTCG_H
