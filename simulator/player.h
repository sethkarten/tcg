#ifndef PLAYER_H
#define PLAYER_H

#include "deck.h"

#define MAX_CARDS_IN_DECK 20
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
    bool energy_available;
    bool cant_retreat;
} Player;

typedef enum {
    PLAY, OPP
} Role;

void initialize_player(Player *player, Role role);
EnergyType get_energy(Player *player, int current_turn);
bool attach_energy(Player *player, EnergyType energy, int target);
void discard_card_from_hand(Player *player, Card *card);
void discard_random_card_from_hand(Player *player);
Card * get_target(Player *player, Player * opponent, int target);
bool move_active_to_hand(Player *player);
void move_lightning_energy_to_active(Player *player);
Card* find_card_in_hand(Player *player, const char *card_name);
void shuffle_active_to_deck(Player *player);

#endif // PLAYER_H
