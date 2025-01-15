#ifndef PTCG_H
#define PTCG_H

#include "game_state.h"

#define MAX_PRIZE_CARDS 3
#define MAX_ENERGY_TYPES 4

#define OBSERVATION_SIZE 92
#define MAX_CARDS 512
#define MAX_DECK_SIZE 20
#define MAX_HAND_SIZE 20
#define MAX_DISCARD_SIZE 20
#define MAX_BENCH_POKEMON 3

// Function prototypes
void init(GameState * game);
void reset(GameState *game, 
                const char player1_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH], 
                int _player1_energy[MAX_CARD_ENERGIES], 
                const char player2_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                int _player2_energy[MAX_CARD_ENERGIES]);
int * get_legal_actions(GameState *game);
int execute_action(GameState *game, int action);
float * get_observation(GameState *game);
int get_current_player(GameState *game);
int is_game_over(GameState *game);
void set_seed_(int seed);
// visualization
char** get_player_hand(GameState* game, int player, int* size);
int get_player_deck_count(GameState* game, int player);
char* get_player_active(GameState* game, int player);
char** get_player_bench(GameState* game, int player, int* size);
int get_player_prizes(GameState* game, int player);

#endif // PTCG_H
