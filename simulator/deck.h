#ifndef DECK_H
#define DECK_H

#include "card.h"

#define MAX_CARDS_IN_DECK 20
#define MAX_DECK_ENERGIES 3

typedef struct {
    Card cards[MAX_CARDS_IN_DECK];
    int card_count;
    bool energy[MAX_CARD_ENERGIES];     // true if energy spawns
} Deck;

void initialize_deck(GameState *game, Deck *deck, const char **card_numbers, bool energy[MAX_CARD_ENERGIES]);
void shuffle_deck(Deck *deck);
void draw_initial_hand(Deck *deck, Player *player);
Card* draw_pokemon_card(Deck *deck, Player *player);
Card* draw_card(Deck *deck, Player *player);
void shuffle_hand_and_draw(Player *player, Deck *deck);

#endif // DECK_H
