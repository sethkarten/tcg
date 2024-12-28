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

