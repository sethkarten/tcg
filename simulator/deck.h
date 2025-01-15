#ifndef DECK_H
#define DECK_H

#include "card.h"
#include "hash.h"

#define MAX_CARDS_IN_DECK 20
#define MAX_DECK_ENERGIES 3
#define MAX_TURN 60

typedef struct {
    Card **cards;
    int card_count;
    int capacity;
    bool *energy;     // true if energy spawns
    EnergyType *energy_seq;     // true if energy spawns
    int energy_seq_count;
} Deck;

void initialize_deck(HashMap *game, Deck *deck, const char (*card_numbers)[MAX_CARD_NAME_LENGTH], bool energy[MAX_CARD_ENERGIES]);;
void shuffle_deck(Deck *deck);
void reset_deck(Deck *deck);
void print_deck(Deck *deck);


#endif // DECK_H
