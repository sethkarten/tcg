#include <stdlib.h>
#include <string.h>
#include "deck.h"
#include "hash.h"

void initialize_deck(HashMap *card_dictionary, Deck *deck, const char (*card_numbers)[MAX_CARD_NAME_LENGTH], bool energy[MAX_CARD_ENERGIES])
{
    deck->card_count = 0;
    for (int i = 0; i < MAX_CARDS_IN_DECK; i++) {
        Card* card = search(card_dictionary, (char*)card_numbers[i]);
        if (card != NULL) {
            deck->cards[deck->card_count] = *card;
            deck->card_count++;
        }
    }
    for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
        deck->energy[i] = energy[i];
    }

    // randomly generate energy sequence
    // Generate energy sequence
    deck->energy_seq_count = MAX_TURN;
    for (int i = 0; i < MAX_TURN; i++) {
        int random_index;
        do {
            random_index = rand() % MAX_CARD_ENERGIES;
        } while (!energy[random_index]);
        
        deck->energy_seq[i] = (EnergyType)random_index;
    }

}

void shuffle_deck(Deck *deck) {
    for (int i = deck->card_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}