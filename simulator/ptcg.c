#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ptcg.h"
#include "supporter.h"

#define INFO_PTCG false

void init(GameState * game)
{
    initialize_game_state(game);
}

void reset(GameState *game, 
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
    reset_game(game, player1_deck, player1_energy, player2_deck, player2_energy);
}

int * get_legal_actions(GameState *game) {
    if (INFO_PTCG) printf("Entering get legal actions.\n");
    int num_actions = 97;
    int *legal_actions = calloc(num_actions, sizeof(int));
    for (int i = 0; i < num_actions; i++) legal_actions[i] = false;
    Player *current_player = get_current_player_(game);
    Player *opponent = get_opponent_(game);

    // forced switched (93-95)
    if (game->turn_effects.sabrina_switch)
    {
        if (INFO_PTCG) printf("Sabrina switch active.\n");
        for (int i = 0; i < current_player->bench_count && i < 3; i++) {
            legal_actions[i + 93] = true;
        }
        return legal_actions;
    }
    if (INFO_PTCG) fflush(stdout);

    // no active? must play a basic
    if (current_player->active_pokemon == NULL)
    {
        if (game->current_turn == 0) 
        {
            for (int i = 0; i < current_player->hand_count; i++) {
                Card *card = current_player->hand[i];
                if (card->cardtype == POKEMON && card->stage == BASIC)  legal_actions[i + 4] = true;
            }
        } else {
            // move from bench
            for (int i = 0; i < current_player->bench_count; i++) 
            {
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
        Card *card = current_player->hand[i];
        if (INFO_PTCG) printf("Hand %d: %s\n", i, card->name);
        if (card->cardtype == POKEMON && card->stage == BASIC && current_player->bench_count < 3) {
            legal_actions[i + 4] = true;
        } else if (card->cardtype == SUPPORTER && !game->supporter_played && game->current_turn != 0 && !opponent_has_shadowy_spellbind(opponent)) {
            if (supporter_is_activatable(game, card)) legal_actions[i + 44] = true;
        } else if (card->cardtype == ITEM && game->current_turn != 0 && item_is_activatable(card, current_player, opponent)) {
            legal_actions[i + 64] = true;
        } else if (card->stage == STAGE1 || card->stage == STAGE2 && game->current_turn != 0)
        {
            // Check active Pokémon for evolution
            if (current_player->active_pokemon != NULL) {
                if (!current_player->active_pokemon->just_played && 
                    card->stage == current_player->active_pokemon->stage + 1 &&
                    strcmp(card->evolves_from, current_player->active_pokemon->name) == 0 &&
                    !opponent_has_primeval_law(opponent))
                {
                    // printf("%d can evolve because %s %s %s\n", i, card->name, card->evolves_from, current_player->active_pokemon->name);
                    // fflush(stdout);
                    legal_actions[i + 24] = legal_actions[i + 24] || true;
                }
            }

            // Check bench Pokémon for evolution
            for (int j = 0; j < current_player->bench_count; j++) {
                if (current_player->bench[j] != NULL) {
                    if (!current_player->bench[j]->just_played && 
                    card->stage == current_player->bench[j]->stage + 1 &&
                    strcmp(card->evolves_from, current_player->bench[j]->name) == 0 &&
                    !opponent_has_primeval_law(opponent))
                    {
                        // printf("%d can evolve because %s %s %s\n", i, card->name, card->evolves_from, current_player->bench[i]->name);
                        // fflush(stdout);
                        legal_actions[i + 24] = legal_actions[i + 24] || true;
                    }
                }
            }
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
    if (current_player->active_pokemon && 
        current_player->active_pokemon->has_ability && 
        !current_player->active_pokemon->ability_used &&
        ability_is_activatable(current_player->active_pokemon)) {
        legal_actions[87] = true;

    }
    for (int j = 0; j < current_player->bench_count && j < 3; j++) {
        if (current_player->bench[j]->has_ability && 
            !current_player->bench[j]->ability_used &&
            ability_is_activatable(current_player->bench[j])) {
            legal_actions[j + 88] = true;
        }
    }

    // Attack (91-92)
    if (current_player->active_pokemon && 
        current_player->active_pokemon->move_count > 0 && 
        current_player->active_pokemon->status != PARALYZED &&
        current_player->active_pokemon->status != ASLEEP) {
        for (int j = 0; j < current_player->active_pokemon->move_count; j++) {
            if (has_enough_energy(current_player, current_player->active_pokemon, &current_player->active_pokemon->moves[j])) {
                legal_actions[91+j] = true;
            }
        }
    }

    // End turn (96)
    legal_actions[96] = true;

    return legal_actions;
}


int execute_action(GameState *game, int action, int target, int opponent_target) {
    if (INFO_PTCG) printf("executing action: %d %d %d\n", action, target, opponent_target);
    if (INFO_PTCG) fflush(stdout);
    Player *current_player = get_current_player_(game);
    Player *opponent = get_opponent_(game);
    int reward = 0;
    char *action_str[4] = {NULL, NULL, NULL, NULL};

    // Allocate memory for each part of action_str
    for (int i = 0; i < 4; i++) {
        action_str[i] = malloc(64 * sizeof(char));
        if (action_str[i] == NULL) {
            // Handle memory allocation failure
            for (int j = 0; j < i; j++) {
                free(action_str[j]);
            }
            return -1;
        }
    }
    if (INFO_PTCG) printf("executing action: allocation ok\n");
    if (INFO_PTCG) fflush(stdout);

    // Format action into string based on get_legal_actions and act_turn
    if (action >= 0 && action <= 3) {
        strcpy(action_str[0], "n");
        strcpy(action_str[1], "");
        snprintf(action_str[2], 20, "%d", action);
        if (INFO_PTCG) printf("Attaching energy to target %d\n", action);
    } else if (action >= 4 && action <= 23) {
        strcpy(action_str[0], "p");
        strncpy(action_str[1], current_player->hand[action - 4]->name, 63);
        action_str[1][63] = '\0';
        if (INFO_PTCG) printf("Playing Pokemon: %s\n", action_str[1]);
    } else if (action >= 24 && action <= 43) {
        strcpy(action_str[0], "v");
        strncpy(action_str[1], current_player->hand[action - 24]->name, 63);
        action_str[1][63] = '\0';
        snprintf(action_str[2], 20, "%d", target);
        if (INFO_PTCG) printf("Evolving Pokemon: %s to target %d\n", action_str[1], target);
    } else if (action >= 44 && action <= 63) {
        strcpy(action_str[0], "s");
        strncpy(action_str[1], current_player->hand[action - 44]->name, 63);
        action_str[1][63] = '\0';
        snprintf(action_str[2], 20, "%d", target);
        if (INFO_PTCG) printf("Playing Supporter card: %s on target %d\n", action_str[1], target);
    } else if (action >= 64 && action <= 83) {
        strcpy(action_str[0], "i");
        strncpy(action_str[1], current_player->hand[action - 64]->name, 63);
        action_str[1][63] = '\0';
        snprintf(action_str[2], 20, "%d", target);
        if (INFO_PTCG) printf("Playing Item card: %s on target %d\n", action_str[1], target);
    } else if (action >= 84 && action <= 86) {
        strcpy(action_str[0], "r");
        if (current_player->active_pokemon) strncpy(action_str[1], current_player->active_pokemon->name, 63);
        else strncpy(action_str[1], "None", 63);
        action_str[1][63] = '\0';
        snprintf(action_str[2], 20, "%d", 1+action-84);
        if (INFO_PTCG) printf("Retreating Pokemon: %s to bench position %d\n", action_str[1], 1+action-84);
    } else if (action >= 87 && action <= 90) {
        strcpy(action_str[0], "a");
        if (action == 87) {
            strncpy(action_str[1], current_player->active_pokemon->name, 63);
        } else {
            strncpy(action_str[1], current_player->bench[action - 88]->name, 63);
        }
        action_str[1][63] = '\0';
        snprintf(action_str[2], 20, "%d", target);
        if (INFO_PTCG) printf("Using ability of Pokemon: %s on target %d\n", action_str[1], target);
    } else if (action == 91 || action == 92) {
        strcpy(action_str[0], "m");
        strncpy(action_str[1], current_player->active_pokemon->name, 63);
        action_str[1][63] = '\0';
        snprintf(action_str[2], 20, "%d", action - 91); // 0 for first move, 1 for second move
        snprintf(action_str[3], 20, "%d", opponent_target);
        if (INFO_PTCG) printf("Using move %d of Pokemon: %s on opponent target %d\n", action - 91, action_str[1], opponent_target);
    } else if (action >= 93 && action <= 95) {
        // switch due to sabrina
        strcpy(action_str[0], "r");
        strncpy(action_str[1], current_player->bench[action - 93]->name, 63);
        action_str[1][63] = '\0';
        snprintf(action_str[2], 20, "%d", action-93+1);
        if (INFO_PTCG) printf("Forced retreat to Pokemon: %s on target %d\n", action_str[1], action-93+1);
        if (INFO_PTCG) fflush(stdout);
    } else if (action == 96) {
        strcpy(action_str[0], "e");
        strncpy(action_str[1], "None", 63);
        action_str[1][63] = '\0';
        if (INFO_PTCG) printf("Ending turn\n");
    }

    // Execute action in game by calling act_turn
    bool action_executed = act_turn(game, current_player, action_str);

    // Check if action was properly executed
    if (!action_executed) {
        reward -= 10; // Punish for executing unavailable actions
        if (INFO_PTCG) printf("Action failed to execute\n");
    } else {
        reward += 1; // Small reward for successful action
        if (INFO_PTCG) printf("Action executed successfully\n");

        // Additional rewards based on game state
        if (current_player->active_pokemon && opponent->active_pokemon) {
            int damage_dealt = opponent->active_pokemon->hp_total - opponent->active_pokemon->hp;
            reward += damage_dealt / 10; // Reward for dealing damage
            if (INFO_PTCG) printf("Damage dealt: %d\n", damage_dealt);
        }

        if (current_player->prize_cards_left < opponent->prize_cards_left) {
            reward += 25; // Reward for taking prize cards
            if (INFO_PTCG) printf("Prize card taken\n");
        }
    }

    // Collect reward by checking for end state
    if (is_game_over(game)) {
        if (game->winner == current_player->role) {
            reward += 100; // Large reward for winning
            if (INFO_PTCG) printf("Game over: Current player wins\n");
        } else {
            reward -= 100; // Large punishment for losing
            if (INFO_PTCG) printf("Game over: Current player loses\n");
        }
    }

     // Free allocated memory
    for (int i = 0; i < 4; i++) {
        free(action_str[i]);
    }

    if (INFO_PTCG) printf("Total reward: %d\n", reward);
    return reward;
}

int* get_valid_targets(GameState *game, int action) {
    int* valid_targets = calloc(8, sizeof(int));
    Player *current_player = get_current_player_(game);
    Player *opponent = get_opponent_(game);

    // Check if the action is valid
    if (action < 0 || action >= 97) {
        return valid_targets;
    }

    // Energy attachment (0-3)
    if (action >= 0 && action <= 3) {
        if (current_player->energy_available && game->current_turn != 0) {
            if (action == 0 && current_player->active_pokemon) {
                valid_targets[0] = 1;
            } else if (action > 0 && action <= 3 && current_player->bench_count >= action) {
                valid_targets[action] = 1;
            }
        }
    }
    // Play a basic Pokemon (4-23)
    else if (action >= 4 && action <= 23) {
        int card_index = action - 4;
        if (card_index < current_player->hand_count) {
            Card *card = current_player->hand[card_index];
            if (card->cardtype == POKEMON && card->stage == BASIC) {
                if (current_player->active_pokemon == NULL) {
                    valid_targets[0] = 1;
                } else if (current_player->bench_count < MAX_BENCH_POKEMON) {
                    for (int i = 1; i <= MAX_BENCH_POKEMON; i++) {
                        valid_targets[i] = 1;
                    }
                }
            }
        }
    }
    // Evolve a Pokemon (24-43)
    else if (action >= 24 && action <= 43) {
        int card_index = action - 24;
        if (card_index < current_player->hand_count) {
            Card *card = current_player->hand[card_index];
            if (card->cardtype == POKEMON && card->stage > BASIC) {
                if (current_player->active_pokemon && strcmp(card->evolves_from, current_player->active_pokemon->name) == 0) {
                    valid_targets[0] = 1;
                }
                for (int i = 0; i < current_player->bench_count; i++) {
                    if (strcmp(card->evolves_from, current_player->bench[i]->name) == 0) {
                        valid_targets[i + 1] = 1;
                    }
                }
            }
        }
    }
    // Play a supporter card (44-63)
    else if (action >= 44 && action <= 63) {
        int card_index = action - 44;
        if (card_index < current_player->hand_count) {
            Card *card = current_player->hand[card_index];
            if (card->cardtype == SUPPORTER && !game->supporter_played && !opponent_has_shadowy_spellbind(opponent)) {
                if (strcmp(card->name, "Erika") == 0) {
                    // Erika targets Grass Pokémon
                    if (current_player->active_pokemon && current_player->active_pokemon->type == GRASS) {
                        valid_targets[0] = true;
                    }
                    for (int i = 0; i < current_player->bench_count; i++) {
                        if (current_player->bench[i]->type == GRASS) {
                            valid_targets[i+1] = true;
                            break;
                        }
                    }
                }
                else if (strcmp(card->name, "Misty") == 0) {
                    // Misty targets Water Pokémon
                    if (current_player->active_pokemon && current_player->active_pokemon->type == WATER) {
                        valid_targets[0] = true;
                    }
                    for (int i = 0; i < current_player->bench_count; i++) {
                        if (current_player->bench[i]->type == WATER) {
                            valid_targets[i+1] = true;
                            break;
                        }
                    }
                }
                else if (strcmp(card->name, "Brock") == 0) {
                    // Brock targets Pokémon with "Geodude", "Graveler", or "Golem" in their name
                    if (current_player->active_pokemon && 
                        (strstr(current_player->active_pokemon->name, "Geodude") ||
                        strstr(current_player->active_pokemon->name, "Graveler") ||
                        strstr(current_player->active_pokemon->name, "Golem"))) {
                        valid_targets[0] = true;
                    }
                    for (int i = 0; i < current_player->bench_count; i++) {
                        if (strstr(current_player->bench[i]->name, "Geodude") ||
                            strstr(current_player->bench[i]->name, "Graveler") ||
                            strstr(current_player->bench[i]->name, "Golem")) {
                            valid_targets[i+1] = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Play an item card (64-83)
    else if (action >= 64 && action <= 83) {
        int card_index = action - 64;
        if (card_index < current_player->hand_count) {
            Card *card = current_player->hand[card_index];
            if (card->cardtype == ITEM) {
                if (strcmp(card->name, "Helix Fossil") == 0 || 
                    strcmp(card->name, "Dome Fossil") == 0 || 
                    strcmp(card->name, "Old Amber") == 0) {
                    if (current_player->bench_count < MAX_BENCH_POKEMON) {
                        valid_targets[0] = 1;
                    }
                }
                else if (strcmp(card->name, "Pokemon Flute") == 0) {
                    for (int i = 0; i < opponent->discard_count; i++) {
                        if (opponent->discard_pile[i]->stage == BASIC) {
                            valid_targets[i + 5] = 1;
                        }
                    }
                }
                else if (strcmp(card->name, "Potion") == 0) {
                    if (current_player->active_pokemon && current_player->active_pokemon->hp < current_player->active_pokemon->hp_total) {
                        valid_targets[0] = 1;
                    }
                    for (int i = 0; i < current_player->bench_count; i++) {
                        if (current_player->bench[i]->hp < current_player->bench[i]->hp_total) {
                            valid_targets[i + 1] = 1;
                        }
                    }
                }
                else if (strcmp(card->name, "X Speed") == 0 ||
                        strcmp(card->name, "Pokedex") == 0 ||
                        strcmp(card->name, "Poke Ball") == 0 ||
                        strcmp(card->name, "Hand Scope") == 0 ||
                        strcmp(card->name, "Red Card") == 0 ||
                        strcmp(card->name, "Mythical Slab") == 0) {
                    valid_targets[0] = 1;
                }
            }
        }
    }

    // Retreat (84-86)
    else if (action >= 84 && action <= 86) {
        if (current_player->active_pokemon && !current_player->cant_retreat) {
            int retreat_cost = current_player->active_pokemon->retreat_cost - game->turn_effects.retreat_reduction;
            if (retreat_cost < 0) retreat_cost = 0;
            if (current_player->active_pokemon->energies_count >= retreat_cost) {
                for (int i = 0; i < current_player->bench_count && i < 3; i++) {
                    valid_targets[i + 1] = 1;
                }
            }
        }
    }
    // Use ability (87-90)
    else if (action >= 87 && action <= 90) {
        if (action == 87 && current_player->active_pokemon && current_player->active_pokemon->has_ability && !current_player->active_pokemon->ability_used) {
            valid_targets[0] = 1;
        } else if (action > 87 && action <= 90) {
            int bench_index = action - 88;
            if (bench_index < current_player->bench_count && current_player->bench[bench_index]->has_ability && !current_player->bench[bench_index]->ability_used) {
                valid_targets[bench_index + 1] = 1;
            }
        }
    }
    // Attack (91-92)
    else if (action >= 91 && action <= 92) {
        if (current_player->active_pokemon && current_player->active_pokemon->move_count > action - 91 &&
            current_player->active_pokemon->status != PARALYZED && current_player->active_pokemon->status != ASLEEP) {
            Move *move = &current_player->active_pokemon->moves[action - 91];
            if (has_enough_energy(current_player, current_player->active_pokemon, move)) {
                valid_targets[4] = 1;  // Opponent's active Pokemon
                for (int i = 0; i < opponent->bench_count; i++) {
                    valid_targets[i + 5] = 1;  // Opponent's bench Pokemon
                }
            }
        }
    }
    // End turn (96)
    else if (action == 96) {
        valid_targets[0] = 1;
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
    if (INFO_PTCG) printf("Getting observation\n");
    if (INFO_PTCG) fflush(stdout);
    float *observation = (float*)calloc(OBSERVATION_SIZE, sizeof(float));
    Player *player = get_current_player_(game);
    print_player_state(player, "current player");
    if (INFO_PTCG) fflush(stdout);
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
            observation[index++] = tokenize_card(player->bench[i]) / (float)MAX_CARDS;
            observation[index++] = player->bench[i]->hp / 100.0f;
            observation[index++] = player->bench[i]->energies_count / 5.0f;
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
            observation[index++] = tokenize_card(opponent->bench[i]) / (float)MAX_CARDS;
            observation[index++] = opponent->bench[i]->hp / 100.0f;
            observation[index++] = opponent->bench[i]->energies_count / 5.0f;
        } else {
            index += 3;
        }
    }

    // Player discard pile (up to 20 cards)
    for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
        if (i < player->discard_count) {
            observation[index++] = tokenize_card(player->discard_pile[i]) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Opponent discard pile (up to 20 cards)
    for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
        if (i < opponent->discard_count) {
            observation[index++] = tokenize_card(opponent->discard_pile[i]) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Player hand (up to 20 cards)
    for (int i = 0; i < MAX_HAND_SIZE; i++) {
        if (i < player->hand_count) {
            observation[index++] = tokenize_card(player->hand[i]) / (float)MAX_CARDS;
        } else {
            index++;
        }
    }

    // Prize cards
    observation[index++] = player->prize_cards_left / 3.0f;
    observation[index++] = opponent->prize_cards_left / 3.0f;

    // Deck sizes
    observation[index++] = player->deck->card_count / (float)MAX_DECK_SIZE;
    observation[index++] = opponent->deck->card_count / (float)MAX_DECK_SIZE;

    // Hand sizes
    observation[index++] = player->hand_count / (float)MAX_HAND_SIZE;
    observation[index++] = opponent->hand_count / (float)MAX_HAND_SIZE;

    // Current turn
    observation[index++] = (game->current_turn) / 50.0f;
    if (INFO_PTCG) printf("last observation\n");
    if (INFO_PTCG) fflush(stdout);

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
