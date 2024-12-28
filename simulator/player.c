#include "player.h"
#include <stdlib.h>

void initialize_player(Player *player, Role role) {
    player->active_pokemon = NULL;
    player->bench_count = 0;
    player->hand_count = 0;
    player->prize_cards_left = 3;
    player->discard_count = 0;
    player->role = role;
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
        printf('Error: invalid target %d\n', target);
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

Card * get_target(Player *player, Player * opponent, int target)
{
    if (target == -1) {
        return NULL;
    } else if (target == 0) {
        return &player->active_pokemon;
    } else if (target <= 3 && target-1 <= player->bench_count) {
        return &player->bench[target-1];
    } else if (target == 4) {
        return &opponent->active_pokemon;
    } else if (target <= 7 && target-1 <= opponent->bench_count) {
        return &opponent->bench[target-1];
    } else {
        printf('Error: invalid target %d\n', target);
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
