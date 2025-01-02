#include "player.h"
#include <stdlib.h>

void initialize_player(Player *player, Role role) {
    player->active_pokemon = NULL;
    player->bench_count = 0;
    player->hand_count = 0;
    player->prize_cards_left = 3;
    player->discard_count = 0;
    player->role = role;
    player->cant_retreat = false;
}

EnergyType get_energy(Player *player, int current_turn)
{
    return player->deck.energy_seq[current_turn];
}

bool attach_energy(Player *player, EnergyType energy, int target)
{
    if (target == 0) 
    {
        player->active_pokemon->attached_energies[(int)energy] += 1;
        player->active_pokemon->energies_count += 1;
    } else if (target <= 3 && target-1 <= player->bench_count) {
        player->bench[target-1].attached_energies[(int)energy] += 1;
        player->bench[target-1].energies_count += 1;
    } else {
        printf("Error: invalid target %d\n", target);
        return false;
    }
    return true;

}

void discard_card_from_hand(Player *player, Card *card) {
    int card_index = -1;
    
    // Find the card in the player's hand
    for (int i = 0; i < player->hand_count; i++) {
        if (&player->hand[i] == card) {
            card_index = i;
            break;
        }
    }
    
    if (card_index == -1) {
        return;  // Card not found in hand
    }

    // Add the card to the discard pile
    player->discard_pile[player->discard_count] = *card;
    player->discard_count++;

    // Remove the card from the hand
    for (int i = card_index; i < player->hand_count - 1; i++) {
        player->hand[i] = player->hand[i + 1];
    }
    player->hand_count--;
}

void discard_random_card_from_hand(Player *player)
{
    if (player->hand_count == 0) {
        return;  // No cards in hand to discard
    }

    // Generate a random index
    int random_index = rand() % player->hand_count;

    // Get the card at the random index
    Card *random_card = &player->hand[random_index];

    // Discard the randomly selected card
    discard_card_from_hand(player, random_card);
}


Card * get_target(Player *player, Player * opponent, int target)
{
    if (target == -1) {
        return NULL;
    } else if (target == 0) {
        return player->active_pokemon;
    } else if (target <= 3 && target-1 <= player->bench_count) {
        return &player->bench[target-1];
    } else if (target == 4) {
        return opponent->active_pokemon;
    } else if (target <= 7 && target-1 <= opponent->bench_count) {
        return &opponent->bench[target-1];
    } else {
        printf("Error: invalid target %d\n", target);
        return NULL;
    }
}

bool move_active_to_hand(Player *player) {
    if (player->active_pokemon == NULL) {
        return false; // No active Pokémon to move
    }

    if (player->hand_count >= MAX_HAND_SIZE) {
        printf("Hand is full. Cannot move active Pokémon to hand.\n");
        return false;
    }

    // Add active Pokémon to hand
    player->hand[player->hand_count] = *player->active_pokemon;
    player->hand_count++;

    // Clear the active Pokémon slot
    free(player->active_pokemon);
    player->active_pokemon = NULL;

    printf("Active Pokémon moved to hand.\n");
    return true;
}

void move_lightning_energy_to_active(Player *player) {
    for (int i = 0; i < player->bench_count; i++) {
        Card *bench_pokemon = &player->bench[i];
        int num_lightning = bench_pokemon->attached_energies[(int)LIGHTNING];
        // Move Lightning energy to active Pokémon
        player->active_pokemon->attached_energies[(int)LIGHTNING] += num_lightning;
        player->active_pokemon->energies_count += num_lightning;
        // Remove energy from bench Pokémon
        bench_pokemon->attached_energies[(int)LIGHTNING] = 0;
        bench_pokemon->energies_count -= num_lightning;
    }
}

Card* find_card_in_hand(Player *player, const char *card_name) {
    for (int i = 0; i < player->hand_count; i++) {
        if (strcmp(player->hand[i].name, card_name) == 0) {
            return &player->hand[i];
        }
    }
    return NULL;
}

void shuffle_active_to_deck(Player *player) {
    if (player->active_pokemon == NULL) {
        return; // No active Pokémon to shuffle
    }

    // Add active Pokémon to the deck
    player->deck.cards[player->deck.card_count] = *player->active_pokemon;
    player->deck.card_count++;

    // Clear the active Pokémon slot
    free(player->active_pokemon);
    player->active_pokemon = NULL;

    // Shuffle the deck
    shuffle_deck(&player->deck);
}

bool opponent_has_primeval_law(Player *opponent) {
    // Check active Pokémon
    if (opponent->active_pokemon && opponent->active_pokemon->has_ability) {
        if (strcmp(opponent->active_pokemon->ability.name, "Primeval Law") == 0) {
            return true;
        }
    }
    
    // Check benched Pokémon
    for (int i = 0; i < opponent->bench_count; i++) {
        if (opponent->bench[i].has_ability) {
            if (strcmp(opponent->bench[i].ability.name, "Primeval Law") == 0) {
                return true;
            }
        }
    }
    
    return false;
}

bool opponent_has_shadowy_spellbind(Player *opponent) {
    // Check active Pokémon
    if (opponent->active_pokemon && opponent->active_pokemon->has_ability) {
        if (strcmp(opponent->active_pokemon->ability.name, "Shadowy Spellbind") == 0) {
            return true;
        }
    }    
    return false;
}

void reset_ability_used(Player *player) {
    // Reset active Pokémon
    if (player->active_pokemon != NULL) {
        player->active_pokemon->ability_used = false;
    }

    // Reset bench Pokémon
    for (int i = 0; i < player->bench_count; i++) {
        player->bench[i].ability_used = false;
    }

}

bool jungle_totem_active(Player *player) {
    // Check active Pokémon
    if (player->active_pokemon && player->active_pokemon->has_ability) {
        if (strcmp(player->active_pokemon->ability.name, "Jungle Totem") == 0) {
            return true;
        }
    }    
    return false;
}

bool has_enough_energy(Player *player, Card *pokemon, Move *move) {
    int required_energy[MAX_CARD_ENERGIES] = {0};
    int available_energy[MAX_CARD_ENERGIES] = {0};
    int colorless_required = 0;
    int total_available = 0;

    // Copy the required energy from the move
    for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
        required_energy[i] = move->energy[i];
        if (i == COLORLESS) {
            colorless_required = move->energy[i];
        }
    }

    // Copy the available energy from the Pokémon
    for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
        available_energy[i] = pokemon->attached_energies[i];
        total_available += pokemon->attached_energies[i];
    }

    // Check if Jungle Totem is active
    bool is_jungle_totem_active = jungle_totem_active(player);

    // Check for specific energy types
    for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
        if (i != COLORLESS) {
            int required = required_energy[i];
            int available = available_energy[i];
            
            if (i == GRASS && is_jungle_totem_active) {
                available *= 2;
            }

            if (available < required) {
                return false;
            }

            available_energy[i] -= required;
            total_available -= required;
        }
    }

    // Check for colorless energy requirement
    if (total_available < colorless_required) {
        return false;
    }

    return true;
}

void draw_initial_hand(Player *player, Deck *deck) {
    // printf("Deck:\n");
    // for (int i = 0; i < deck->card_count; i++) {
    //     printf("%d. %s\n", i + 1, deck->cards[i].name);
    // }
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

    // printf("Player's hand:\n");
    // for (int i = 0; i < player->hand_count; i++) {
    //     printf("%d. %s\n", i + 1, player->hand[i].name);
    // }
}

Card* draw_pokemon_card(Player *player, Deck *deck) {
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

Card* draw_card(Player *player, Deck *deck) {
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

