#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "player.h"
#include "card.h"
#include "hash.h"

#define MAX_CARDS 512

typedef struct {
    int retreat_reduction;
    bool blaine_boost;
    bool giovanni_boost;
    bool sabrina_switch;
    bool blue_protection;
    bool must_flip_before_attack;
} TurnEffects;

typedef struct {
    Player player1;
    Player player2;
    int current_turn;
    bool supporter_played;
    HashMap* card_dictionary;
    bool game_over;
    Player * winner;
    TurnEffects turn_effects;
    Role current_player;
} GameState;

void initialize_game_state(GameState *game, 
                           const char deck1[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                           bool energy1[MAX_CARD_ENERGIES],
                           const char deck2[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                           bool energy2[MAX_CARD_ENERGIES]
);

void start_turn(GameState *game, Player *player);
bool act_turn(GameState *game, Player *player, char ** action);

bool play_item(GameState *game, Player *player, char *card_name, int target);
bool play_pokemon(GameState *game, Player *player, char *card_name);
bool evolve_pokemon(GameState *game, Player *player, char *card_name, int target);
bool retreat_pokemon(GameState *game, Player *player, char *card_name, int target);
bool use_ability(GameState *game, Player *player, char *card_name, int target);
bool use_move(GameState *game, Player *player, char *card_name, int move_index, int opponent_target);
void check_for_KO(GameState *game, Player *player, Player * opponent, Card *opponent_card);
bool end_turn(GameState *game, Player *player);
Player * get_winner(GameState *game);
Player * get_current_player_(GameState *game);
Player * get_opponent_(GameState *game);
void print_player_state(Player *player, const char *player_name);
const char* energy_type_to_string(EnergyType type);

#endif // GAME_STATE_H
