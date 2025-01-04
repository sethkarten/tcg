#include <stdlib.h>
#include <string.h>
#include "deck.h"
#include "hash.h"

#define DECK_DEBUG false

void initialize_deck(HashMap *card_dictionary, Deck *deck, const char (*card_numbers)[MAX_CARD_NAME_LENGTH], bool energy[MAX_CARD_ENERGIES])
{
    deck->card_count = 0;
    deck->capacity = MAX_CARDS_IN_DECK;
    deck->cards = (Card**)malloc(deck->capacity * sizeof(Card*));
    if (deck->cards == NULL) {
        fprintf(stderr, "Memory allocation failed for deck cards\n");
        return;
    }

    for (int i = 0; i < MAX_CARDS_IN_DECK; i++) {
        Card* card = search(card_dictionary, (char*)card_numbers[i]);
        if (card != NULL) {
            deck->cards[deck->card_count] = (Card*)malloc(sizeof(Card));
            if (deck->cards[deck->card_count] == NULL) {
                fprintf(stderr, "Memory allocation failed for card in deck\n");
                continue;
            }
            memcpy(deck->cards[deck->card_count], card, sizeof(Card));
            deck->card_count++;
        }
    }

    for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
        deck->energy[i] = energy[i];
    }

    int valid_energy_count = 0;
    EnergyType valid_energies[MAX_CARD_ENERGIES];

    for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
        if (energy[i]) {
            valid_energies[valid_energy_count] = (EnergyType)i;
            valid_energy_count++;
        }
    }

    for (int i = 0; i < MAX_TURN; i++) {
        if (valid_energy_count > 0) {
            int random_index = rand() % valid_energy_count;
            deck->energy_seq[i] = valid_energies[random_index];
        } else {
            deck->energy_seq[i] = GRASS;
        }
    }
}


void shuffle_deck(Deck *deck) {
    for (int i = deck->card_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card *temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}

void reset_deck(Deck *deck) {
    for (int i = 0; i < deck->card_count; i++) {
        if (DECK_DEBUG) printf("freeing deck.c:L65\n");
        free(deck->cards[i]);
    }
        if (DECK_DEBUG) printf("freeing deck.c:L68\n");
    free(deck->cards);
    deck->cards = NULL;
    deck->card_count = 0;
    deck->capacity = 0;
}
