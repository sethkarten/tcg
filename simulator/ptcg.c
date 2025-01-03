#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ptcg.h"

void reset_game(GameState *game, 
                const char player1_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH], 
                int _player1_energy[MAX_CARD_ENERGIES], 
                const char player2_deck[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                int _player2_energy[MAX_CARD_ENERGIES]) {
    bool player1_energy[MAX_CARD_ENERGIES];
    bool player2_energy[MAX_CARD_ENERGIES];
    for (int i=0; i < MAX_CARD_ENERGIES; i++)
    {
        player1_energy[i] = _player1_energy[i] == 1;
        player2_energy[i] = _player2_energy[i] == 1;
    }
    initialize_game_state(game, player1_deck, player1_energy, player2_deck, player2_energy);
}

int * get_legal_actions(GameState *game) {
    int num_actions = 97;
    int *legal_actions = calloc(num_actions, sizeof(int));
    for (int i = 0; i < num_actions; i++) legal_actions[i] = false;
    Player *current_player = get_current_player_(game);
    Player *opponent = get_opponent_(game);

    // forced switched (93-95)
    if (game->turn_effects.sabrina_switch)
    {
        for (int i = 0; i < current_player->bench_count && i < 3; i++) {
                legal_actions[i + 93] = true;
            }
        return legal_actions;
    }

    // no active? must play a basic
    if (current_player->active_pokemon == NULL)
    {
        if (game->current_turn == 0) 
        {
            for (int i = 0; i < current_player->hand_count; i++) {
                Card *card = &current_player->hand[i];
                if (card->cardtype == POKEMON && card->stage == BASIC)  legal_actions[i + 4] = true;
            }
        } else {
            // move from bench
            printf("bench?\n");
            for (int i = 0; i < current_player->bench_count; i++) 
            {
                printf("yes\n");
                legal_actions[i + 84] = true;
            }
        }
        
        return legal_actions;
    }

    // Use energy on active (0)
    if (current_player->energy_available && current_player->active_pokemon && game->current_turn != 0) {
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
        printf("Hand %d: %s\n", i, card->name);
        if (card->cardtype == POKEMON && card->stage == BASIC && current_player->bench_count < 3) {
            legal_actions[i + 4] = true;
        } else if (strcmp(card->evolves_from, current_player->active_pokemon->name) == 0) {
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

    // Attack (91-92)
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

    // End turn (95)
    legal_actions[96] = true;

    return legal_actions;
}


int execute_action(GameState *game, int action, int target, int opponent_target) {
    Player *current_player = get_current_player_(game);
    Player *opponent = get_opponent_(game);
    int reward = 0;
    char *action_str[4] = {NULL, NULL, NULL, NULL};

    // Format action into string based on get_legal_actions and act_turn
    if (action >= 0 && action <= 3) {
        action_str[0] = "n";
        action_str[1] = "";
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
        printf("Attaching energy to target %d\n", target);
    } else if (action >= 4 && action <= 23) {
        action_str[0] = "p";
        action_str[1] = current_player->hand[action - 4].name;
        printf("Playing Pokemon: %s\n", action_str[1]);
    } else if (action >= 24 && action <= 43) {
        action_str[0] = "v";
        action_str[1] = current_player->hand[action - 24].name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
        printf("Evolving Pokemon: %s to target %d\n", action_str[1], target);
    } else if (action >= 44 && action <= 63) {
        action_str[0] = "s";
        action_str[1] = current_player->hand[action - 44].name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
        printf("Playing Supporter card: %s on target %d\n", action_str[1], target);
    } else if (action >= 64 && action <= 83) {
        action_str[0] = "i";
        action_str[1] = current_player->hand[action - 64].name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
        printf("Playing Item card: %s on target %d\n", action_str[1], target);
    } else if (action >= 84 && action <= 86) {
        action_str[0] = "r";
        action_str[1] = current_player->active_pokemon->name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", 1+action-84);
        printf("Retreating Pokemon: %s to bench position %d\n", action_str[1], 1+action-84);
    } else if (action >= 87 && action <= 90) {
        action_str[0] = "a";
        if (action == 87) {
            action_str[1] = current_player->active_pokemon->name;
        } else {
            action_str[1] = current_player->bench[action - 88].name;
        }
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
        printf("Using ability of Pokemon: %s on target %d\n", action_str[1], target);
    } else if (action == 91 || action == 92) {
        action_str[0] = "m";
        action_str[1] = current_player->active_pokemon->name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", action - 91); // 0 for first move, 1 for second move
        action_str[3] = malloc(10);
        snprintf(action_str[3], 10, "%d", opponent_target);
        printf("Using move %d of Pokemon: %s on opponent target %d\n", action - 91, action_str[1], opponent_target);
    } else if (action >= 93 && action <= 95) {
        action_str[0] = "s";
        action_str[1] = current_player->active_pokemon->name;
        action_str[2] = malloc(10);
        snprintf(action_str[2], 10, "%d", target);
        printf("Using special action of Pokemon: %s on target %d\n", action_str[1], target);
    } else if (action == 96) {
        action_str[0] = "e";
        printf("Ending turn\n");
    }

    // Execute action in game by calling act_turn
    bool action_executed = act_turn(game, current_player, action_str);

    // Check if action was properly executed
    if (!action_executed) {
        reward -= 10; // Punish for executing unavailable actions
        printf("Action failed to execute\n");
    } else {
        reward += 1; // Small reward for successful action
        printf("Action executed successfully\n");

        // Additional rewards based on game state
        if (current_player->active_pokemon && opponent->active_pokemon) {
            int damage_dealt = opponent->active_pokemon->hp_total - opponent->active_pokemon->hp;
            reward += damage_dealt / 10; // Reward for dealing damage
            printf("Damage dealt: %d\n", damage_dealt);
        }

        if (current_player->prize_cards_left < opponent->prize_cards_left) {
            reward += 25; // Reward for taking prize cards
            printf("Prize card taken\n");
        }
    }

    // Collect reward by checking for end state
    if (is_game_over(game)) {
        if (game->winner == current_player) {
            reward += 100; // Large reward for winning
            printf("Game over: Current player wins\n");
        } else {
            reward -= 100; // Large punishment for losing
            printf("Game over: Current player loses\n");
        }
    }

    // Free allocated memory
    for (int i = 0; i < 4; i++) {
        if (action_str[i] && (i == 2 || i == 3)) {
            free(action_str[i]);
        }
    }

    printf("Total reward: %d\n", reward);
    return reward;
}

int* get_valid_targets(GameState *game, int action) {
    int num_targets = 8;
    int *valid_targets = calloc(num_targets, sizeof(int));
    for (int i = 0; i < num_targets; i++) {
        valid_targets[i] = false;
    }

    Player *current_player = get_current_player_(game);
    Player *opponent = get_opponent_(game);

    if (action >= 0 && action <= 3) {
        // Energy attachment
        valid_targets[0] = (current_player->active_pokemon != NULL);
        for (int i = 0; i < current_player->bench_count && i < 3; i++) {
            valid_targets[i + 1] = true;
        }
    } else if (action >= 24 && action <= 43) {
        // Evolution
        Card *evolution_card = &current_player->hand[action - 24];
        valid_targets[0] = (current_player->active_pokemon && strcmp(current_player->active_pokemon->name, evolution_card->evolves_from) == 0);
        for (int i = 0; i < current_player->bench_count && i < 3; i++) {
            valid_targets[i + 1] = (strcmp(current_player->bench[i].name, evolution_card->evolves_from) == 0);
        }
    } else if (action >= 44 && action <= 63) {
        // Supporter cards
        Card *supporter_card = &current_player->hand[action - 44];
        // Set valid targets based on the specific supporter card
        // This is a simplified example and should be expanded based on actual supporter card effects
        for (int i = 0; i < 8; i++) {
            valid_targets[i] = true;
        }
    } else if (action >= 64 && action <= 83) {
        // Item cards
        Card *item_card = &current_player->hand[action - 64];
        // Set valid targets based on the specific item card
        // This is a simplified example and should be expanded based on actual item card effects
        for (int i = 0; i < 8; i++) {
            valid_targets[i] = true;
        }
    } else if (action >= 87 && action <= 90) {
        // Ability use
        // This is a simplified example and should be expanded based on specific ability effects
        for (int i = 0; i < 8; i++) {
            valid_targets[i] = true;
        }
    }

    return valid_targets;
}

int* get_valid_opponent_target(GameState *game, int action) {
    int num_targets = 8;
    int *valid_targets = calloc(num_targets, sizeof(int));
    for (int i = 0; i < num_targets; i++) {
        valid_targets[i] = false;
    }

    Player *current_player = get_current_player_(game);
    Player *opponent = get_opponent_(game);

    if (action == 91 || action == 92) {
        // Attack moves
        valid_targets[0] = (opponent->active_pokemon != NULL);
        for (int i = 0; i < opponent->bench_count && i < 3; i++) {
            valid_targets[4 + i + 1] = true;
        }
        valid_targets[0] = (current_player->active_pokemon != NULL);
        for (int i = 0; i < current_player->bench_count && i < 3; i++) {
            valid_targets[i + 1] = true;
        }
    }

    return valid_targets;
}


float* get_observation(GameState *game) {
    float *observation = (float*)calloc(OBSERVATION_SIZE, sizeof(float));
    printf("Getting observation\n");
    Player *player = get_current_player_(game);
    Player *opponent = get_opponent_(game);
    int index = 0;

    // Player active Pokémon
    if (player->active_pokemon) {
        observation[index++] = tokenize_card(player->active_pokemon) / (float)MAX_CARDS;
        observation[index++] = player->active_pokemon->hp / 100.0f;
        observation[index++] = player->active_pokemon->energies_count / 5.0f;
    } else {
        index += 3;
    }

    // Player bench (up to 3 Pokémon)
    for (int i = 0; i < MAX_BENCH_POKEMON; i++) {
        if (i < player->bench_count) {
            observation[index++] = tokenize_card(&player->bench[i]) / (float)MAX_CARDS;
            observation[index++] = player->bench[i].hp / 100.0f;
            observation[index++] = player->bench[i].energies_count / 5.0f;
        } else {
            index += 3;
        }
    }

    // Opponent active Pokémon
    if (opponent->active_pokemon) {
        observation[index++] = tokenize_card(opponent->active_pokemon) / (float)MAX_CARDS;
        observation[index++] = opponent->active_pokemon->hp / 100.0f;
        observation[index++] = opponent->active_pokemon->energies_count / 5.0f;
    } else {
        index += 3;
    }

    // Opponent bench (up to 3 Pokémon)
    for (int i = 0; i < MAX_BENCH_POKEMON; i++) {
        if (i < opponent->bench_count) {
            observation[index++] = tokenize_card(&opponent->bench[i]) / (float)MAX_CARDS;
            observation[index++] = opponent->bench[i].hp / 100.0f;
            observation[index++] = opponent->bench[i].energies_count / 5.0f;
        } else {
            index += 3;
        }
    }

    // Player discard pile (up to 20 cards)
    for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
        if (i < player->discard_count) {
            observation[index++] = tokenize_card(&player->discard_pile[i]) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Opponent discard pile (up to 20 cards)
    for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
        if (i < opponent->discard_count) {
            observation[index++] = tokenize_card(&opponent->discard_pile[i]) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Player hand (up to 20 cards)
    for (int i = 0; i < MAX_HAND_SIZE; i++) {
        if (i < player->hand_count) {
            observation[index++] = tokenize_card(&player->hand[i]) / (float)MAX_CARDS;
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
    observation[index++] = (game->current_turn) / 50.0f;
    printf("last observation %d\n", index);

    // Available actions
    // bool* legal_actions = get_legal_actions(game, NULL);
    // for (int i = 0; i < 95; i++) {
    //     observation[index++] = legal_actions[i] ? 1.0f : 0.0f;
    // }
    // free(legal_actions);

    return observation;
}

int get_current_player(GameState *game)
{
    return game->current_player;
}

int is_game_over(GameState *game) 
{
    return game->game_over;
}
