#include <stdlib.h>
#include <string.h>
#include "deck.h"
#include "hash.h"

void initialize_deck(GameState *game, Deck *deck, const char **card_numbers, bool energy[MAX_CARD_ENERGIES]) {
    deck->card_count = 0;
    for (int i = 0; i < MAX_CARDS_IN_DECK; i++) {
        Card* card = search(game->card_dictionary, card_numbers[i]);
        if (card != NULL) {
            deck->cards[deck->card_count] = *card;
            deck->card_count++;
        }
    }
    for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
        deck->energy[i] = energy[i];
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

void draw_initial_hand(Deck *deck, Player *player) {
    int basic_pokemon_index = -1;
    int cards_drawn = 0;

    // Draw the first 4 cards
    for (int i = 0; i < 4; i++) {
        if (deck->card_count > 0) {
            player->hand[player->hand_count++] = deck->cards[--deck->card_count];
            cards_drawn++;
            
            // Check if a BASIC Pokémon was drawn
            if (player->hand[player->hand_count - 1].stage == BASIC) {
                basic_pokemon_index = i;
            }
        }
    }

    // If no BASIC Pokémon was drawn, find the first one in the deck
    if (basic_pokemon_index == -1) {
        for (int i = deck->card_count - 1; i >= 0; i--) {
            if (deck->cards[i].stage == BASIC) {
                basic_pokemon_index = i;
                break;
            }
        }

        // If a BASIC Pokémon was found in the deck, add it to the hand
        if (basic_pokemon_index != -1) {
            player->hand[player->hand_count++] = deck->cards[basic_pokemon_index];
            cards_drawn++;

            // Remove the BASIC Pokémon from the deck
            for (int i = basic_pokemon_index; i < deck->card_count - 1; i++) {
                deck->cards[i] = deck->cards[i + 1];
            }
            deck->card_count--;
        }
    }

    // Draw the final card if needed
    if (cards_drawn < 5 && deck->card_count > 0) {
        player->hand[player->hand_count++] = deck->cards[--deck->card_count];
    }
}

Card* draw_pokemon_card(Deck *deck, Player *player) {
    if (deck->card_count == 0) {
        return NULL; // Deck is empty
    }

    for (int i = deck->card_count - 1; i >= 0; i--) {
        Card drawn_card = deck->cards[i];
        
        // Remove the card from the deck
        for (int j = i; j < deck->card_count - 1; j++) {
            deck->cards[j] = deck->cards[j + 1];
        }
        deck->card_count--;

        // Add the card to the player's hand
        player->hand[player->hand_count] = drawn_card;
        player->hand_count++;

        return &player->hand[player->hand_count - 1];
    }

    return NULL; // No Pokémon card found in the deck
}

Card* draw_card(Deck *deck, Player *player) {
    if (deck->card_count == 0) {
        return NULL; // Deck is empty
    }

    // Draw the top card from the deck
    Card drawn_card = deck->cards[deck->card_count - 1];
    deck->card_count--;

    // Add the card to the player's hand
    player->hand[player->hand_count] = drawn_card;
    player->hand_count++;

    return &player->hand[player->hand_count - 1];
}

void shuffle_hand_and_draw(Player *player, Deck *deck) {
    // Add cards from hand to deck
    for (int i = 0; i < player->hand_count; i++) {
        deck->cards[deck->card_count] = player->hand[i];
        deck->card_count++;
    }
    player->hand_count = 0;

    // Shuffle the deck
    shuffle_deck(deck);

    // Draw 3 cards
    for (int i = 0; i < 3 && deck->card_count > 0; i++) {
        player->hand[player->hand_count] = deck->cards[deck->card_count - 1];
        player->hand_count++;
        deck->card_count--;
    }
}

