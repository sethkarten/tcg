#ifndef PLAYER_H
#define PLAYER_H

#include "deck.h"

#define MAX_CARDS_IN_DECK 20
#define MAX_HAND_SIZE 20
#define MAX_BENCH_POKEMON 3
#define MAX_PRIZE_CARDS 3

typedef enum {
    PLAY, OPP, TIE, NONE_ROLE,
} Role;

typedef struct {
    // pokemon
    Card *active_pokemon;
    Card **bench;
    int bench_count;
    int bench_capacity;

    // deck
    Deck *deck;

    // hand
    Card **hand;
    int hand_count;
    int hand_capacity;

    // win con
    int prize_cards_left;

    // discard pile
    Card **discard_pile;
    int discard_count;
    int discard_capacity;

    // misc
    Role role;
    bool energy_available;
    bool cant_retreat;
} Player;


void initialize_player(Player *player, Role role);
void reset_player(Player *player);
void cleanup_player(Player *player);

EnergyType get_energy(Player *player, int current_turn);
bool attach_energy(Player *player, EnergyType energy, int target);
void discard_card_from_hand(Player *player, Card *card);
void discard_random_card_from_hand(Player *player);
Card * get_target(Player *player, Player * opponent, int target);
bool move_active_to_hand(Player *player);
void move_lightning_energy_to_active(Player *player);
Card* find_card_in_hand(Player *player, const char *card_name);
void shuffle_active_to_deck(Player *player);
bool opponent_has_primeval_law(Player *opponent);
bool opponent_has_shadowy_spellbind(Player *opponent);
void reset_ability_used(Player *player);
bool jungle_totem_active(Player *player);
bool has_enough_energy(Player *player, Card *pokemon, Move *move);
void draw_initial_hand(Player *player, Deck *deck);
bool draw_pokemon_card(Player *player, Deck *deck);
Card* draw_card(Player *player, Deck *deck);
void shuffle_hand_and_draw(Player *player, Deck *deck);
void reset_just_played(Player *player);

#endif // PLAYER_H
