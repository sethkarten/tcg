#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ptcg.h"

void reset_game(GameState *game, 
                const char player1_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH], 
                bool player1_energy[MAX_CARD_ENERGIES], 
                const char player2_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                 bool player2_energy[MAX_CARD_ENERGIES]) {
    initialize_game_state(game, player1_deck, player1_energy, player2_deck, player2_energy);
}

bool * get_legal_actions(GameState *game, int *actions) {
    bool *legal_actions = calloc(95, sizeof(bool));
    Player *current_player = (game->current_turn % 2 == 0) ? &game->player1 : &game->player2;
    Player *opponent = (current_player == &game->player1) ? &game->player2 : &game->player1;

    // forced switched
    if (game->turn_effects.sabrina_switch)
    {
        for (int i = 0; i < current_player->bench_count && i < 3; i++) {
                legal_actions[i + 92] = true;
            }
        return legal_actions;
    }
    // Use energy on active (0)
    if (current_player->energy_available && current_player->active_pokemon) {
        legal_actions[0] = true;
    }

    // Use energy on bench pokemon (1-3)
    for (int i = 0; i < current_player->bench_count && i < 3; i++) {
        if (current_player->energy_available) {
            legal_actions[i + 1] = true;
        }
    }

    // Play a basic pokemon (4-23)
    // Evolve a pokemon (24-43)
    // Play a supporter card (44-63)
    // Play an item card (64-83)
    for (int i = 0; i < current_player->hand_count; i++) {
        Card *card = &current_player->hand[i];
        if (card->cardtype == POKEMON && card->stage == BASIC) {
            legal_actions[i + 4] = true;
        } else if (card->cardtype == POKEMON && card->stage > BASIC) {
            legal_actions[i + 24] = true;
        } else if (card->cardtype == SUPPORTER && !game->supporter_played) {
            legal_actions[i + 44] = true;
        } else if (card->cardtype == ITEM) {
            legal_actions[i + 64] = true;
        }
    }

    // Retreat (84-86)
    if (current_player->active_pokemon && !current_player->cant_retreat) {
        int retreat_cost = current_player->active_pokemon->retreat_cost - game->turn_effects.retreat_reduction;
        if (retreat_cost < 0) retreat_cost = 0;
        if (current_player->active_pokemon->energies_count >= retreat_cost) {
            for (int i = 0; i < current_player->bench_count && i < 3; i++) {
                legal_actions[i + 84] = true;
            }
        }
    }

    // Use ability (87-90)
    if (current_player->active_pokemon && current_player->active_pokemon->has_ability && !current_player->active_pokemon->ability_used) {
        legal_actions[87] = true;
    }
    for (int i = 0; i < current_player->bench_count && i < 3; i++) {
        if (current_player->bench[i].has_ability && !current_player->bench[i].ability_used) {
            legal_actions[i + 88] = true;
        }
    }

    // Attack (91)
    if (current_player->active_pokemon && 
        current_player->active_pokemon->move_count > 0 && 
        current_player->active_pokemon->status != PARALYZED &&
        current_player->active_pokemon->status != ASLEEP) {
        for (int i = 0; i < current_player->active_pokemon->move_count; i++) {
            if (has_enough_energy(current_player, current_player->active_pokemon, &current_player->active_pokemon->moves[i])) {
                legal_actions[91+i] = true;
            }
        }
    }

    // End turn (90)
    legal_actions[90] = true;

    return legal_actions;
}


int execute_action(GameState *game, int action, int target, int opponent_target) {
    Player *current_player = (game->current_turn % 2 == 0) ? &game->player1 : &game->player2;
    Player *opponent = (game->current_turn % 2 == 0) ? &game->player2 : &game->player1;
    int reward = 0;
    char *action_str[4] = {NULL, NULL, NULL, NULL};

    // Format action into string based on get_legal_actions and act_turn
    if (action >= 0 && action <= 3) {
        action_str[0] = "n";
        action_str[1] = "";
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
    } else if (action >= 4 && action <= 23) {
        action_str[0] = "p";
        action_str[1] = current_player->hand[action - 4].name;
    } else if (action >= 24 && action <= 43) {
        action_str[0] = "v";
        action_str[1] = current_player->hand[action - 24].name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
    } else if (action >= 44 && action <= 63) {
        action_str[0] = "s";
        action_str[1] = current_player->hand[action - 44].name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
    } else if (action >= 64 && action <= 83) {
        action_str[0] = "i";
        action_str[1] = current_player->hand[action - 64].name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
    } else if (action >= 84 && action <= 86) {
        action_str[0] = "r";
        action_str[1] = current_player->active_pokemon->name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
    } else if (action >= 87 && action <= 90) {
        action_str[0] = "a";
        if (action == 87) {
            action_str[1] = current_player->active_pokemon->name;
        } else {
            action_str[1] = current_player->bench[action - 88].name;
        }
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
    } else if (action == 91 || action == 92) {
        action_str[0] = "m";
        action_str[1] = current_player->active_pokemon->name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", action - 91); // 0 for first move, 1 for second move
        action_str[3] = malloc(10);
        snprintf(action_str[3], 10, "%d", opponent_target);
    } else if (action >= 92 && action <= 94) {
        action_str[0] = "s";
        action_str[1] = current_player->active_pokemon->name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
    } else if (action == 90) {
        action_str[0] = "e";
    }

    // Execute action in game by calling act_turn
    bool action_executed = act_turn(game, current_player, action_str);

    // Check if action was properly executed
    if (!action_executed) {
        reward -= 10; // Punish for executing unavailable actions
    } else {
        reward += 1; // Small reward for successful action

        // Additional rewards based on game state
        if (current_player->active_pokemon && opponent->active_pokemon) {
            int damage_dealt = opponent->active_pokemon->hp_total - opponent->active_pokemon->hp;
            reward += damage_dealt / 10; // Reward for dealing damage
        }

        if (current_player->prize_cards_left < opponent->prize_cards_left) {
            reward += 25; // Reward for taking prize cards
        }
    }

    // Collect reward by checking for end state
    if (is_game_over(game)) {
        if (game->winner == current_player) {
            reward += 100; // Large reward for winning
        } else {
            reward -= 100; // Large punishment for losing
        }
    }

    // Free allocated memory
    for (int i = 0; i < 4; i++) {
        if (action_str[i] && (i == 2 || i == 3)) {
            free(action_str[i]);
        }
    }

    return reward;
}

// Card tokenizer function
int tokenize_card(const char* card_name) {
    // This function should return a unique integer for each card name
    // You would need to maintain a mapping of card names to integers
    // For simplicity, we'll use a hash function here
    int hash = 0;
    for (int i = 0; card_name[i] != '\0'; i++) {
        hash = 31 * hash + card_name[i];
    }
    return (hash % MAX_CARDS) + 1; // Ensure non-zero token
}

float* get_observation(GameState *game) {
    float *observation = (float*)calloc(OBSERVATION_SIZE, sizeof(float));
    Player *player = (game->current_turn % 2 == 0) ? &game->player1 : &game->player2;
    Player *opponent = (game->current_turn % 2 == 0) ? &game->player2 : &game->player1;
    int index = 0;

    // Player active Pokémon
    if (player->active_pokemon) {
        observation[index++] = tokenize_card(player->active_pokemon->name) / (float)MAX_CARDS;
        observation[index++] = player->active_pokemon->hp / 100.0f;
        observation[index++] = player->active_pokemon->energies_count / 5.0f;
    } else {
        index += 3;
    }

    // Player bench (up to 3 Pokémon)
    for (int i = 0; i < MAX_BENCH_POKEMON; i++) {
        if (i < player->bench_count) {
            observation[index++] = tokenize_card(player->bench[i].name) / (float)MAX_CARDS;
            observation[index++] = player->bench[i].hp / 100.0f;
            observation[index++] = player->bench[i].energies_count / 5.0f;
        } else {
            index += 3;
        }
    }

    // Opponent active Pokémon
    if (opponent->active_pokemon) {
        observation[index++] = tokenize_card(opponent->active_pokemon->name) / (float)MAX_CARDS;
        observation[index++] = opponent->active_pokemon->hp / 100.0f;
        observation[index++] = opponent->active_pokemon->energies_count / 5.0f;
    } else {
        index += 3;
    }

    // Opponent bench (up to 3 Pokémon)
    for (int i = 0; i < MAX_BENCH_POKEMON; i++) {
        if (i < opponent->bench_count) {
            observation[index++] = tokenize_card(opponent->bench[i].name) / (float)MAX_CARDS;
            observation[index++] = opponent->bench[i].hp / 100.0f;
            observation[index++] = opponent->bench[i].energies_count / 5.0f;
        } else {
            index += 3;
        }
    }

    // Player discard pile (up to 20 cards)
    for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
        if (i < player->discard_count) {
            observation[index++] = tokenize_card(player->discard_pile[i].name) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Opponent discard pile (up to 20 cards)
    for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
        if (i < opponent->discard_count) {
            observation[index++] = tokenize_card(opponent->discard_pile[i].name) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Player hand (up to 20 cards)
    for (int i = 0; i < MAX_HAND_SIZE; i++) {
        if (i < player->hand_count) {
            observation[index++] = tokenize_card(player->hand[i].name) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Prize cards
    observation[index++] = player->prize_cards_left / 3.0f;
    observation[index++] = opponent->prize_cards_left / 3.0f;

    // Deck sizes
    observation[index++] = player->deck.card_count / (float)MAX_DECK_SIZE;
    observation[index++] = opponent->deck.card_count / (float)MAX_DECK_SIZE;

    // Hand sizes
    observation[index++] = player->hand_count / (float)MAX_HAND_SIZE;
    observation[index++] = opponent->hand_count / (float)MAX_HAND_SIZE;

    // Current turn
    observation[index++] = (game->current_turn % 2) / 1.0f;

    // Available actions
    bool* legal_actions = get_legal_actions(game, NULL);
    for (int i = 0; i < 95; i++) {
        observation[index++] = legal_actions[i] ? 1.0f : 0.0f;
    }
    free(legal_actions);

    return observation;
}


bool is_game_over(GameState *game) {
    return game->game_over;
}
