#ifndef DECK_H
#define DECK_H

#include "card.h"
#include "player.h"

#define MAX_CARDS_IN_DECK 20
#define MAX_DECK_ENERGIES 3
#define MAX_TURN 60

typedef struct {
    Card cards[MAX_CARDS_IN_DECK];
    int card_count;
    bool energy[MAX_CARD_ENERGIES];     // true if energy spawns
    EnergyType energy_seq[MAX_TURN];     // true if energy spawns
    int energy_seq_count;
} Deck;

void initialize_deck(HashMap *game, Deck *deck, const char **card_numbers, bool energy[MAX_CARD_ENERGIES]);
void shuffle_deck(Deck *deck);
void draw_initial_hand(Deck *deck, Player *player);
Card* draw_pokemon_card(Deck *deck, Player *player);
Card* draw_card(Deck *deck, Player *player);
void shuffle_hand_and_draw(Player *player, Deck *deck);

#endif // DECK_H
