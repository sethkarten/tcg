#include "game_state.h"
#include "data.h"
#include "utils.h"
#include "player.h"
#include "supporter.h"

#include <time.h>

#define GAME_STATE_DEBUG false

void initialize_game_state(GameState *game)
{
    // Initialize random seed
    // Load card data from JSON
    game->card_dictionary = (HashMap*)malloc(sizeof(HashMap));
    load_card_data_from_json(game, "pokemon_tcg_pocket_cards.json");
    if (GAME_STATE_DEBUG) printf("here1\n");
    
    // Init players
    initialize_player(&game->player1, PLAY);
    initialize_player(&game->player2, OPP);
    if (GAME_STATE_DEBUG) printf("Players init\n");

}

void reset_game(GameState * game,
                const char deck1[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                bool energy1[MAX_CARD_ENERGIES],
                const char deck2[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                bool energy2[MAX_CARD_ENERGIES])
{
    if (GAME_STATE_DEBUG) printf("\n\n\nResetting game.\n");
    // Iterate through each bucket in the hash table
    for (int i = 0; i < game->card_dictionary->capacity; i++) {
        struct node *current = game->card_dictionary->arr[i];
        
        // Traverse the linked list at this bucket
        while (current != NULL) {
            Card *card = current->value;

            // Reset attached energies
            if (card->attached_energies != NULL) {
                memset(card->attached_energies, 0, sizeof(int) * card->energies_count);
                card->energies_count = 0;
            }

            // Reset other relevant fields
            card->hp = card->hp_total;                      // Reset HP to full
            card->status = NONE_STATUS;                            // Reset status effects
            card->prevent_damage_next_turn = false;         // Reset damage prevention flag
            card->ability_used = false;                     // Reset ability usage flag

            // Move to the next node in the linked list
            current = current->next;
        }
    }
    reset_player(&game->player1);
    reset_player(&game->player2);

    if (GAME_STATE_DEBUG) printf("here2\n");
    // Initialize player 1's deck
    initialize_deck(game->card_dictionary, game->player1.deck, deck1, energy1);
    if (GAME_STATE_DEBUG) print_deck(game->player1.deck);

    if (GAME_STATE_DEBUG) printf("here3\n");
    // Initialize player 2's deck
    initialize_deck(game->card_dictionary, game->player2.deck, deck2, energy2);
    if (GAME_STATE_DEBUG) print_deck(game->player2.deck);

    // Shuffle decks
    shuffle_deck(game->player1.deck);
    shuffle_deck(game->player2.deck);

    // Initialize other game state variables
    game->winner = NONE_ROLE;
    game->current_turn = 0;
    game->supporter_played = false;
    game->game_over = false;
    game->turn_effects.blaine_boost = false;
    game->turn_effects.giovanni_boost = false;
    game->turn_effects.sabrina_switch = false;
    game->turn_effects.blue_protection = false;
    game->turn_effects.retreat_reduction = 0;
    game->current_player = PLAY;

    // Draw initial hands
    draw_initial_hand(&game->player1, game->player1.deck);
    draw_initial_hand(&game->player2, game->player2.deck);

    if (GAME_STATE_DEBUG) printf("Initial game state:\n");
    print_player_state(&game->player1, "Player 1");
    print_player_state(&game->player2, "Player 2");
    if (GAME_STATE_DEBUG) printf("Finished reset.\n");
    if (GAME_STATE_DEBUG) fflush(stdout);
}


void start_turn(GameState *game, Player *player)
{
    if (GAME_STATE_DEBUG) printf("Starting turn.\n");
    Player *opponent = get_opponent_(game);

    if (GAME_STATE_DEBUG) printf("\nStarting turn for %s:\n", (player == &game->player1) ? "Player 1" : "Player 2");
    print_player_state(player, (player == &game->player1) ? "Player 1" : "Player 2");
    print_player_state(opponent, (opponent == &game->player2) ? "Player 2" : "Player 1");
    if (GAME_STATE_DEBUG) fflush(stdout);
    if (game->current_turn == 0) return;
    // if not turn 1
    if (game->current_turn != 1)
    {
        player->energy_available = true;
    }
    draw_card(player, player->deck);
    game->turn_effects.blaine_boost = false;
    game->turn_effects.giovanni_boost = false;
    game->turn_effects.retreat_reduction = 0;
    reset_ability_used(player);

    // checkup on status effects
    // ASLEEP
    if (player->active_pokemon) {
        if (player->active_pokemon->status == ASLEEP) {
            if (flip_coin() == HEADS) {
                player->active_pokemon->status = NONE_STATUS;
            }
        }
    }
    if (opponent->active_pokemon) {
        if (opponent->active_pokemon->status == ASLEEP) {
            if (flip_coin() == HEADS) {
                opponent->active_pokemon->status = NONE_STATUS;
            }
        }
    }
    if (GAME_STATE_DEBUG) printf("Finished starting turn.\n");
    if (GAME_STATE_DEBUG) fflush(stdout);
}

bool act_turn(GameState *game, Player *player, char **action) 
{
    char *action_type = action[0];
    char *card_name = (action[1] != NULL) ? action[1] : "";
    int target = (action[2] != NULL) ? atoi(action[2]) : -1;
    int opponent_target = (action[3] != NULL) ? atoi(action[3]) : -1;
    int turn_number = (int) game->current_turn / 2;
    if (GAME_STATE_DEBUG) printf("act turn: %s %s %d %d %d\n", action_type, card_name, target, opponent_target, turn_number);

    bool success = true;
    Player *opponent = get_opponent_(game);
    switch(action_type[0]) {
        case 'n':
            if (player->energy_available) 
            {
                EnergyType energy = get_energy(player, turn_number);
                return attach_energy(player, energy, target);
            }
            return false;
            break;
        case 's':
            success = play_supporter(game, player, card_name, target);
            break;
        case 'i':
            success = play_item(game, player, card_name, target);
            break;
        case 'p':
            success = play_pokemon(game, player, card_name);
            break;
        case 'v':
            success = evolve_pokemon(game, player, card_name, target);
            break;
        case 'r':
            success = retreat_pokemon(game, player, card_name, target);
            break;
        case 'a':
            success = use_ability(game, player, card_name, target);
            break;
        case 'm':
            success = use_move(game, player, card_name, target, opponent_target);
            if (!success) break;
            end_turn(game, player);
            break;
        case 'e':
            end_turn(game, player);
            break;
        default:
            if (GAME_STATE_DEBUG) printf("Invalid action type: %c\n", action_type[0]);
    }
    if (success && GAME_STATE_DEBUG) printf("Action successful\n");
    else if (GAME_STATE_DEBUG) printf("Action NOT successful\n");
    if (GAME_STATE_DEBUG) printf("\nAfter action:\n");
    print_player_state(player, (player == &game->player1) ? "Player 1" : "Player 2");
    print_player_state(opponent, (opponent == &game->player2) ? "Player 2" : "Player 1");

    return success;
}



bool play_item(GameState *game, Player *player, char *card_name, int target) {
    Card *card = find_card_in_hand(player, card_name);
    if (card == NULL || card->cardtype != ITEM) {
        return false;
    }

    Player *opponent = get_opponent_(game);

    if (strcmp(card_name, "Helix Fossil") == 0 ||
        strcmp(card_name, "Dome Fossil") == 0 ||
        strcmp(card_name, "Old Amber") == 0) {
        // Play this card as if it were a 40-HP Basic Pokémon.
        if (player->bench_count < MAX_BENCH_POKEMON) {
            Card *fossil = card;
            fossil->hp = 40;
            fossil->hp_total = 40;
            fossil->stage = BASIC;
            fossil->type = COLORLESS;
            card->retreat_cost = 0;
            player->bench[player->bench_count++] = fossil;
        } else {
            return false;
        }
    } else if (strcmp(card_name, "Pokemon Flute") == 0) {
        // Put a Basic Pokémon from your opponent's discard pile onto their Bench.
        Card *basic_pokemon = NULL;
        if (target < opponent->discard_count) {
            if (opponent->discard_pile[target]->stage == BASIC) {
                basic_pokemon = opponent->discard_pile[target];
            } else {
                return false;
            }
        } else {
            return false;
        }
        if (basic_pokemon && opponent->bench_count < MAX_BENCH_POKEMON) {
            opponent->bench[opponent->bench_count++] = basic_pokemon;
            // Remove from discard pile
            for (int i = 0; i < opponent->discard_count - 1; i++) {
                opponent->discard_pile[i] = opponent->discard_pile[i + 1];
            }
            opponent->discard_count--;
        } else {
            return false;
        }
    } else if (strcmp(card_name, "Mythical Slab") == 0) {
        // Look at the top card of your deck-> If that card is a Pokémon, put it into your hand. If it is not a Pokémon, put it on the bottom of your deck->
        if (player->deck->card_count > 0) {
            Card *top_card = player->deck->cards[player->deck->card_count - 1];
            if (top_card->cardtype == POKEMON) {
                player->hand[player->hand_count++] = top_card;
            } else {
                player->deck->cards[0] = top_card;
                for (int i = player->deck->card_count - 1; i > 0; i--) {
                    player->deck->cards[i] = player->deck->cards[i - 1];
                }
            }
            player->deck->card_count--;
        } else {
            return false;
        }
    } else if (strcmp(card_name, "Potion") == 0) {
        // Heal 20 damage from 1 of your Pokémon.
        Card *target_pokemon = get_target(player, opponent, target);
        if (target_pokemon) {
            heal_card(target_pokemon, 20);
        } else {
            return false;
        }
    } else if (strcmp(card_name, "X Speed") == 0) {
        // During this turn, the Retreat Cost of your Active Pokémon is 1 less.
        game->turn_effects.retreat_reduction++;
    } else if (strcmp(card_name, "Pokedex") == 0) {
        // Look at the top 3 cards of your deck->
        // This effect is mostly for the player's information, so we'll just print the cards
        // @todo: change from print to get obs information
        for (int i = 0; i < 3 && i < player->deck->card_count; i++) {
            if (GAME_STATE_DEBUG) printf("Card %d: %s\n", i + 1, player->deck->cards[player->deck->card_count - 1 - i]->name);
        }
    } else if (strcmp(card_name, "Poke Ball") == 0) {
        // Put 1 random Basic Pokémon from your deck into your hand.
        bool basic_pokemon = draw_pokemon_card(player, player->deck);
        if (!basic_pokemon) return false;
    } else if (strcmp(card_name, "Hand Scope") == 0) {
        // Your opponent reveals their hand.
        // This effect is mostly for the player's information, so we'll just print the opponent's hand
        // @todo: change from print to get obs information
        if (GAME_STATE_DEBUG) printf("Opponent's hand:\n");
        for (int i = 0; i < opponent->hand_count; i++) {
            if (GAME_STATE_DEBUG) printf("%s\n", opponent->hand[i]->name);
        }
    } else if (strcmp(card_name, "Red Card") == 0) {
        // Your opponent shuffles their hand into their deck and draws 3 cards.
        for (int i = 0; i < opponent->hand_count; i++) {
            opponent->deck->cards[opponent->deck->card_count++] = opponent->hand[i];
        }
        opponent->hand_count = 0;
        shuffle_deck(opponent->deck);
        for (int i = 0; i < 3 && opponent->deck->card_count > 0; i++) {
            opponent->hand[opponent->hand_count++] = opponent->deck->cards[--opponent->deck->card_count];
        }
    } else {
        return false;
    }

    discard_card_from_hand(player, card);
    return true;
}

bool item_is_activatable(Card *card, Player *player, Player *opponent) {
    if (card == NULL || card->cardtype != ITEM) {
        return false;
    }

    const char *card_name = card->name;

    if (strcmp(card_name, "Helix Fossil") == 0 ||
        strcmp(card_name, "Dome Fossil") == 0 ||
        strcmp(card_name, "Old Amber") == 0) {
        if (GAME_STATE_DEBUG) printf("bench count %d\n", player->bench_count);
        return player->bench_count < MAX_BENCH_POKEMON;
    }
    else if (strcmp(card_name, "Pokemon Flute") == 0) {
        return opponent->discard_count > 0 && opponent->bench_count < MAX_BENCH_POKEMON;
    }
    else if (strcmp(card_name, "Mythical Slab") == 0) {
        return player->deck->card_count > 0;
    }
    else if (strcmp(card_name, "Potion") == 0) {
        // Check if any Pokémon (active or on bench) is missing HP
        if (player->active_pokemon && player->active_pokemon->hp < player->active_pokemon->hp_total) {
            return true;
        }
        for (int i = 0; i < player->bench_count; i++) {
            if (player->bench[i]->hp < player->bench[i]->hp_total) {
                return true;
            }
        }
        return false;
    }
    else if (strcmp(card_name, "X Speed") == 0) {
        return player->active_pokemon != NULL;
    }
    else if (strcmp(card_name, "Pokedex") == 0) {
        return player->deck->card_count > 0;
    }
    else if (strcmp(card_name, "Poke Ball") == 0) {
        return player->deck->card_count > 0;
    }
    else if (strcmp(card_name, "Hand Scope") == 0) {
        return opponent->hand_count > 0;
    }
    else if (strcmp(card_name, "Red Card") == 0) {
        return opponent->hand_count > 0 || opponent->deck->card_count > 0;
    }

    return false;
}



bool play_pokemon(GameState *game, Player *player, char *card_name) {
    Card *card = find_card_in_hand(player, card_name);
    if (card != NULL && card->cardtype == POKEMON && card->stage == BASIC) {
        if (player->active_pokemon == NULL) {
            // Play to active spot if it's empty
            player->active_pokemon = (Card *) malloc(sizeof(Card));
            if (player->active_pokemon == NULL) {
                return false; // Memory allocation failed
            }
            *player->active_pokemon = *card;
        } else if (player->bench_count < MAX_BENCH_POKEMON) {
            // Play to bench if there's space
            player->bench[player->bench_count] = card;
            player->bench_count++;
        } else {
            return false; // No space to play the Pokémon
        }

        // Remove card from hand
        for (int i = 0; i < player->hand_count; i++) {
            if (player->hand[i] == card) {
                for (int j = i; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                break;
            }
        }
        card->just_played = true;
        return true;
    }
    return false;
}


bool evolve_pokemon(GameState *game, Player *player, char *card_name, int target) {
    Player *opponent = get_opponent_(game);
    Card *evolution = find_card_in_hand(player, card_name);
    Card *target_pokemon = get_target(player, NULL, target);
    if (GAME_STATE_DEBUG) printf("Evolve pokemon: %s\n", card_name);
    if (GAME_STATE_DEBUG) fflush(stdout);
    if (evolution != NULL && target_pokemon != NULL && 
        strcmp(evolution->evolves_from, target_pokemon->name) == 0 &&
        !opponent_has_primeval_law(opponent)) {
        
        // Save the attached energies and damage
        int attached_energies[MAX_CARD_ENERGIES];
        memcpy(attached_energies, target_pokemon->attached_energies, sizeof(attached_energies));
        int energies_count = target_pokemon->energies_count;
        int current_damage = target_pokemon->hp_total - target_pokemon->hp;

        // Evolve the target Pokemon
        *target_pokemon = *evolution;
        
        // Restore attached energies and apply damage
        memcpy(target_pokemon->attached_energies, attached_energies, sizeof(attached_energies));
        target_pokemon->energies_count = energies_count;
        target_pokemon->hp = target_pokemon->hp_total - current_damage;

        // Remove evolution card from hand
        for (int i = 0; i < player->hand_count; i++) {
            if (player->hand[i] == evolution) {
                for (int j = i; j < player->hand_count - 1; j++) {
                    player->hand[j] = player->hand[j + 1];
                }
                player->hand_count--;
                break;
            }
        }

        return true;
    }
    
    return false;
}

bool retreat_pokemon(GameState *game, Player *player, char *card_name, int target) {
    if (GAME_STATE_DEBUG) printf("retreat_pokemon %d\n", target);
    if (GAME_STATE_DEBUG) fflush(stdout);
    Card *active = player->active_pokemon;
    if (active == NULL)
    {
        if (target < 0 || target > player->bench_count)
        {
            if (GAME_STATE_DEBUG) printf("Invalid target for sending out Pokemon\n");
            if (GAME_STATE_DEBUG) fflush(stdout);
            return false;
        }
        // Sending out pokemon from bench
        player->active_pokemon = get_target(player, NULL, target);
        
        // Shift the remaining bench Pokemon
        for (int i = target-1; i < player->bench_count - 1; i++)
        {
            player->bench[i] = player->bench[i + 1];
        }
        
        player->bench_count--;
        
        // Clear the last bench slot
        player->bench[player->bench_count] = NULL;

        if (GAME_STATE_DEBUG) printf("Putting out new pokemon: %s\n", player->active_pokemon->name);
        if (GAME_STATE_DEBUG) fflush(stdout);
        return true;
    }

    if (target < 0 || target > player->bench_count || player->cant_retreat) {
        if (target < 0) {
            if (GAME_STATE_DEBUG) printf("Cannot retreat: Invalid target (less than 0).\n");
        }
        if (target >= player->bench_count) {
            if (GAME_STATE_DEBUG) printf("Cannot retreat: Target (%d) exceeds bench count (%d).\n", target, player->bench_count);
        }
        if (player->cant_retreat) {
            if (GAME_STATE_DEBUG) printf("Cannot retreat: Player is prevented from retreating.\n");
        }
        if (GAME_STATE_DEBUG) fflush(stdout);
        return false;
    }

    Card *bench_pokemon = get_target(player, NULL, target);

    // Calculate retreat cost
    int retreat_cost = active->retreat_cost - game->turn_effects.retreat_reduction;
    if (retreat_cost < 0) retreat_cost = 0;

    // Check if enough energy for retreat
    int total_energy = active->energies_count;

    if (game->turn_effects.sabrina_switch) {
        retreat_cost = -1;
        game->turn_effects.sabrina_switch = false;
    }

    if (total_energy < retreat_cost) {
        // printf("Condition not met to retreat: retreat cost.\n");
        return false;
    }

    // Remove energy used for retreat
    int energy_removed = 0;
    for (int i = 0; i < MAX_CARD_ENERGIES && energy_removed < retreat_cost; i++) {
        while (active->attached_energies[i] > 0 && energy_removed < retreat_cost) {
            active->attached_energies[i]--;
            active->energies_count--;
            energy_removed++;
        }
    }

    // Remove any status effects
    player->active_pokemon->status = NONE_STATUS;

    // Swap active and bench Pokemon
    Card temp = *active;
    *active = *bench_pokemon;
    *bench_pokemon = temp;

    return true;
}

bool use_ability(GameState *game, Player *player, char *card_name, int target) {
    Card *card = find_card_in_hand(player, card_name);
    if (card == NULL || !card->has_ability) {
        return false;
    }

    Player *opponent = get_opponent_(game);

    if (strcmp(card->ability->name, "Powder Heal") == 0 && !card->ability_used) {
        for (int i = 0; i < player->bench_count+1; i++) {
            Card *target_card = get_target(player, opponent, i);
            if (target_card)
            {
                heal_card(target_card, 20);
                card->ability_used = true;
            }
        }
        return true;
    }

    else if (strcmp(card->ability->name, "Fragrance Trap") == 0 && !card->ability_used) {
        if (card != player->active_pokemon) {
            return false;
        }
        Card *target_card = get_target(player, opponent, target);
        if (target_card) {
            Card temp = *opponent->active_pokemon;
            *opponent->active_pokemon = *target_card;
            *target_card = temp;
            card->ability_used = true;
            return true;
        }
        return false;
    }

    if (strcmp(card->ability->name, "Water Shuriken") == 0 && !card->ability_used) {
        Card *target_card = get_target(player, opponent, target);
        if (target_card) {
            target_card->hp -= 20;
            check_for_KO(game, player, opponent, target_card);
            card->ability_used = true;
            return true;
        }
        return false;
    }

    if (strcmp(card->ability->name, "Volt Charge") == 0 && !card->ability_used) {
        attach_energy_to_card(card, LIGHTNING);
        card->ability_used = true;
        return true;
    }

    if (strcmp(card->ability->name, "Sleep Pendulum") == 0 && !card->ability_used) {
        if (flip_coin() == HEADS) {
            opponent->active_pokemon->status = ASLEEP;
            card->ability_used = true;
            return true;
        }
        return false;
    }

    if (strcmp(card->ability->name, "Psy Shadow") == 0 && !card->ability_used) {
        if (player->deck->energy[PSYCHIC] && player->active_pokemon && player->active_pokemon->type == PSYCHIC) {
            attach_energy_to_card(player->active_pokemon, PSYCHIC);
            card->ability_used = true;
            return true;
        }
        return false;
    }

    if (strcmp(card->ability->name, "Gas Leak") == 0 && !card->ability_used) {
        if (card == player->active_pokemon) {
            opponent->active_pokemon->status = POISONED;
            card->ability_used = true;
            return true;
        }
        return false;
    }

    if (strcmp(card->ability->name, "Drive Off") == 0 && !card->ability_used) {
        if (opponent->bench_count > 0) {
            game->turn_effects.sabrina_switch = true;
            card->ability_used = true;
            return true;
        }
        return false;
    }

    if (strcmp(card->ability->name, "Data Scan") == 0 && !card->ability_used) {
        if (player->deck->card_count > 0) {
            // In a real game, we would show this card to the player
            // Here we'll just return true to indicate the ability was used
            card->ability_used = true;
            return true;
        }
        return false;
    }

    if (strcmp(card->ability->name, "Wash Out") == 0) {
        Card *source = get_target(player, NULL, target);
        if (source && source->type == WATER && source != player->active_pokemon) {
            if (source->attached_energies[WATER] > 0) {
                source->attached_energies[WATER]--;
                source->energies_count--;
                player->active_pokemon->attached_energies[WATER]++;
                player->active_pokemon->energies_count++;
                return true;
            }
        }
        return false;
    }

    return false;
}

bool ability_is_activatable(Card *card) {
    if (card == NULL || !card->has_ability || !card->ability_used) {
        return false;
    }

    if (strcmp(card->ability->name, "Powder Heal") == 0 ||
        strcmp(card->ability->name, "Fragrance Trap") == 0 ||
        strcmp(card->ability->name, "Water Shuriken") == 0 ||
        strcmp(card->ability->name, "Volt Charge") == 0 ||
        strcmp(card->ability->name, "Sleep Pendulum") == 0 || 
        strcmp(card->ability->name, "Psy Shadow") == 0 ||
        strcmp(card->ability->name, "Gas Leak") == 0 || 
        strcmp(card->ability->name, "Drive Off") == 0 ||
        strcmp(card->ability->name, "Data Scan") == 0 ||
        strcmp(card->ability->name, "Wash Out") == 0) {
        return true;
        }

    return false;
}


bool use_move(GameState *game, Player *player, char *card_name, int move_index, int opponent_target) {
    Player *opponent = get_opponent_(game);
    Card *active = player->active_pokemon;
    Card *opponent_card = opponent->active_pokemon;

    if (active == NULL || 
        opponent_card == NULL || 
        move_index < 0 || 
        move_index >= active->move_count || 
        active->status == PARALYZED ||
        active->status == ASLEEP) {
            if (GAME_STATE_DEBUG) printf("use move not executing.\n");
        return false;
    }

    Move *move = &active->moves[move_index];

    // Check if enough energy for move
    if (!has_enough_energy(player, active, move)) 
    {
        // printf("Not enough energies");
        return false;
    }

    // Calculate base damage
    int damage = move->damage;

    // Apply Blaine and Giovanni boosts
    if (game->turn_effects.blaine_boost && (strcmp(active->name, "Ninetales") == 0 || 
        strcmp(active->name, "Rapidash") == 0 || strcmp(active->name, "Magmar") == 0)) {
        damage += 30;
    }
    if (game->turn_effects.giovanni_boost) {
        damage += 10;
    }

    // Apply Blue's protection
    if (game->turn_effects.blue_protection) {
        damage -= 10;
        if (damage < 0) damage = 0;
    }
    if (strcmp(opponent_card->ability->name, "Shell Armor") == 0) {
        damage -= 10;
        if (damage < 0) damage = 0;
    }
        if (strcmp(opponent_card->ability->name, "Hard Coat") == 0) {
        damage -= 20;
        if (damage < 0) damage = 0;
    }

    // All-or-Nothing
    if (game->turn_effects.must_flip_before_attack == true) {
        game->turn_effects.must_flip_before_attack = false;
        if (flip_coin() == TAILS) {
            // printf("Attack does nothing");
            return true; // Attack does nothing, but turn is used
        }
    }

    // printf("checking description\n");
    if (move->description != NULL) {
        // Copy opponent's attack
        if (strstr(move->description, "Choose 1 of your opponent's Pokemon's attacks and use it as this attack") != NULL) {
            Card *target = get_target(player, opponent, opponent_target);
            if (target && target->move_count > 0) {
                int chosen_move = rand() % target->move_count;
                Move *copied_move = &target->moves[chosen_move];
                
                // Check if this Pokemon has enough energy for the copied move
                bool has_energy = has_enough_energy(player, active, copied_move);
                
                if (has_energy) {
                    move = copied_move;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (strstr(move->description, "Choose 1 of your opponent's Active Pokemon's attacks and use it as this attack") != NULL) {
            // Copy opponent's active Pokemon's attack
            if (opponent->active_pokemon && opponent->active_pokemon->move_count > 0) {
               move = &opponent->active_pokemon->moves[(int) (opponent_target % opponent->active_pokemon->move_count)];
            } else {
                return false;
            }
        }
    }

    // Apply move effects
    if (move->description != NULL) {
        // Damage Boost
        if (strstr(move->description, "Flip a coin. If heads, this attack does") != NULL) {
            int additional_damage = 0;
            sscanf(strstr(move->description, "does ") + 5, "%d", &additional_damage);
            if (flip_coin() == HEADS) {
                damage += additional_damage;
            }
        }

        // Conditional Effect
        if (strstr(move->description, "Flip a coin. If heads,") != NULL) {
            if (flip_coin() == HEADS) {
                if (strstr(move->description, "Paralyzed") != NULL) {
                    opponent_card->status = PARALYZED;
                } else if (strstr(move->description, "Asleep") != NULL) {
                    opponent_card->status = ASLEEP;
                } else if (strstr(move->description, "Poisoned") != NULL) {
                    opponent_card->status = POISONED;
                } else if (strstr(move->description, "prevent all damage") != NULL) {
                    active->prevent_damage_next_turn = true;
                } else if (strstr(move->description, "discard a random Energy") != NULL) {
                    discard_random_energy(opponent_card);
                } else if (strstr(move->description, "discard a random card") != NULL) {
                    discard_random_card_from_hand(opponent);
                } else if (strstr(move->description, "shuffles their Active Pokemon back") != NULL) {
                    shuffle_active_to_deck(opponent);
                    return true;
                }
            } else {
                // Self-damage effect
                if (strstr(move->description, "this Pokemon also does 20 damage to itself") != NULL) {
                    active->hp -= 20;
                    if (active->hp < 0) active->hp = 0;
                }
            }
        } else {
            // Status effects without coin flips
            if (strstr(move->description, "Asleep") != NULL) {
                opponent_card->status = ASLEEP;
            }
            if (strstr(move->description, "Paralyzed") != NULL) {
                opponent_card->status = PARALYZED;
            }
            if (strstr(move->description, "Poisoned") != NULL) {
                opponent_card->status = POISONED;
            }
            // Self-damage effect
            if (strstr(move->description, "this Pokemon also does 20 damage to itself") != NULL) {
                active->hp -= 20;
                if (active->hp < 0) active->hp = 0;
            }
        }

        // Multiple Flips
        if (strstr(move->description, "Flip") != NULL && strstr(move->description, "coins") != NULL) {
            int num_coins = 0;
            sscanf(move->description, "Flip %d coins", &num_coins);
            int heads_count = flip_coins(num_coins);
            
            if (strstr(move->description, "damage for each heads") != NULL) {
                int damage_per_heads = 0;
                sscanf(strstr(move->description, "does ") + 5, "%d", &damage_per_heads);
                damage = heads_count * damage_per_heads;
            }

            else if (strstr(move->description, "80 more damage") != NULL && heads_count == 2) {
                damage += 80;
            }

            else if (strstr(move->description, "Flip 3 coins. Take an amount of Fire Energy from your Energy Zone") != NULL) {
                int target = opponent_target % 4;
                if (heads_count > 0 && target > 0) {
                    if (player->bench_count > target-1) {
                        for (int i = 0; i < heads_count; i++) {
                            attach_energy_to_card(player->bench[target-1], FIRE);
                        }
                    } else return false;
                } else {
                    return false;
                    if (GAME_STATE_DEBUG) printf("No benched Fire Pokémon to attach energy to.\n");
                }
            }
        }

        // Flip Until Tails
        if (strstr(move->description, "Flip a coin until you get tails") != NULL) {
            int heads_count = 0;
            while (flip_coin() == HEADS) {
                heads_count++;
            }
            if (strstr(move->description, "damage for each heads") != NULL) {
                int damage_per_heads = 0;
                sscanf(strstr(move->description, "does ") + 5, "%d", &damage_per_heads);
                damage = heads_count * damage_per_heads;
            }
        }

        // All-or-Nothing
        if (strstr(move->description, "If tails, this attack does nothing") != NULL) {
            if (flip_coin() == TAILS) {
                return true; // Attack does nothing, but turn is used
            }
        }

        // Energy discard effects
        if (strstr(move->description, "Discard") != NULL && strstr(move->description, "Energy") != NULL) {
            int discard_count = 0;
            EnergyType energy_type;
            if (strstr(move->description, "2 Fire Energy") != NULL) {
                discard_count = 2;
                energy_type = FIRE;
            } else if (strstr(move->description, "a Fire Energy") != NULL || strstr(move->description, "1 Fire Energy") != NULL) {
                discard_count = 1;
                energy_type = FIRE;
            } else if (strstr(move->description, "2 Psychic Energy") != NULL) {
                discard_count = 2;
                energy_type = PSYCHIC;
            }

            for (int i = 0; i < discard_count; i++) {
                if (active->attached_energies[energy_type] > 0) {
                    active->attached_energies[energy_type]--;
                    active->energies_count--;
                }
            }
        }

        // Single target attack
        if (strstr(move->description, "This attack does") != NULL && strstr(move->description, "damage to 1 of your opponent's Pokemon") != NULL) {
            int damage_move = 0;
            sscanf(move->description, "This attack does %d damage to 1 of your opponent's Pokemon", &damage_move);
            
            Card *target = get_target(player, opponent, opponent_target);
            if (target) {
                if (!target->prevent_damage_next_turn) 
                {
                    if (opponent_card == opponent->active_pokemon &&
                        (strcmp(opponent_card->ability->name, "Counterattack") == 0 || 
                        strcmp(opponent_card->ability->name, "Rough Skin") == 0)) 
                    {
                        active->hp -= 20;
                        check_for_KO(game, opponent, player, active);
                    }
                    target->hp -= damage_move;
                    check_for_KO(game, player, opponent, target);
                }
            }
            return true;
        }

        // Random multi-target attack
        if (strstr(move->description, "1 of your opponent's Pokemon is chosen at random 4 times") != NULL) {
            int targets[4] = {0}; // Index 0 for active, 1-3 for bench
            int damages[4] = {0};
            int total_targets = opponent->bench_count + 1;

            // Select targets and calculate damage
            for (int i = 0; i < 4; i++) {
                int random_target = rand() % total_targets;
                targets[random_target]++;
                damages[random_target] += 50;
            }

            // Apply damage and check for KO
            if (damages[0] > 0) {
                opponent->active_pokemon->hp -= damages[0];
                check_for_KO(game, player, opponent, opponent->active_pokemon);
            }

            for (int i = 0; i < opponent->bench_count && i < 3; i++) {
                if (damages[i + 1] > 0) {
                    opponent->bench[i]->hp -= damages[i + 1];
                    check_for_KO(game, player, opponent, opponent->bench[i]);
                }
            }

            return true;
        }

        // Healing effects
        if (strstr(move->description, "Heal") != NULL) {
            int heal_amount = 0;
            sscanf(move->description, "Heal %d damage", &heal_amount);
            heal_card(active, heal_amount);
        }

        // Additional damage based on opponent's energy
        if (strstr(move->description, "30 more damage for each Energy") != NULL) {
            damage += 30 * opponent_card->energies_count;
        }

        // Prevent retreat effects
        if (strstr(move->description, "can't retreat") != NULL) {
            opponent->cant_retreat = true;
        }

        // Draw card effects
        if (strstr(move->description, "Draw") != NULL) {
            int draw_count = 0;
            sscanf(move->description, "Draw %d card", &draw_count);
            for (int i = 0; i < draw_count; i++) {
                draw_card(player, player->deck);
            }
        }

        // Flip effect for opponent's next turn
        if (strstr(move->description, "During your opponent's next turn, if the Defending Pokemon tries to use an attack") != NULL) {
            game->turn_effects.must_flip_before_attack = true;
        }
    }

    // check for weakness
    if (active->type == opponent_card->weakness) damage += 20;

    // Apply damage
    if (!opponent_card->prevent_damage_next_turn) 
    {
        if (strcmp(opponent_card->ability->name, "Counterattack") == 0 || 
            strcmp(opponent_card->ability->name, "Rough Skin") == 0) 
        {
            active->hp -= 20;
            check_for_KO(game, opponent, player, active);
        }
        opponent_card->hp -= damage;
    }
    if (opponent_card) opponent_card->prevent_damage_next_turn = false;
    // printf("damage applied\n");

    // Check for KO
    check_for_KO(game, player, opponent, opponent_card);

    return true;
}

void check_for_KO(GameState *game, Player *player, Player * opponent, Card *opponent_card)
{
    // Check if opponent's Pokémon is knocked out
    if (opponent_card->hp <= 0) {
        opponent_card->hp = 0;
        int prize_cards_taken = opponent_card->is_ex ? 2 : 1;
        opponent->prize_cards_left -= prize_cards_taken;
        if (opponent->prize_cards_left <= 0) {
            game->game_over = true;
            game->winner = player->role;
        }
        // Move knocked out Pokémon to discard pile
        if (opponent_card == opponent->active_pokemon) {
            opponent->discard_pile[opponent->discard_count++] = opponent_card;
            // printf("%s knocked out.\n", opponent->active_pokemon->name);
            opponent->active_pokemon = NULL;
            if (opponent->bench_count == 0) {
                game->game_over = true;
                game->winner = player->role;
            }
        } else {
            for (int i = 0; i < opponent->bench_count; i++) {
                if (opponent->bench[i] == opponent_card) {
                    opponent->discard_pile[opponent->discard_count++] = opponent->bench[i];
                    for (int j = i; j < opponent->bench_count - 1; j++) {
                        opponent->bench[j] = opponent->bench[j + 1];
                    }
                    opponent->bench_count--;
                    break;
                }
            }
        }
    }
    return;
}

bool end_turn(GameState *game, Player *player) {
    Player *opponent = get_opponent_(game);
    if (game->current_turn == 0 && game->current_player == PLAY)
    {
        game->current_player = OPP;
    } else {
        if (GAME_STATE_DEBUG) printf("Inc turn %d -> %d\n\n", game->current_turn, game->current_turn+1);
        game->current_turn++;
        game->current_player = (game->current_turn % 2 == 0) ? PLAY : OPP;
        start_turn(game, get_current_player_(game));
    }
    game->supporter_played = false;
    player->cant_retreat = false;

    // remove paralysis at end of turn
    if (player->active_pokemon->status == PARALYZED) player->active_pokemon->status = NONE_STATUS;

    // Poison status
    if (player->active_pokemon && player->active_pokemon->status == POISONED) {
        player->active_pokemon->hp -= 10;
        check_for_KO(game, player, opponent, player->active_pokemon);
    }
    if (opponent->active_pokemon && opponent->active_pokemon->status == POISONED) {
        opponent->active_pokemon->hp -= 10;
        check_for_KO(game, opponent, player, opponent->active_pokemon);
    }
    
    if (GAME_STATE_DEBUG) printf("End of turn %d\n\n", game->current_turn);
    if (GAME_STATE_DEBUG) fflush(stdout);
    // if (opponent->role == OPP) printf("Opponent's turn.\n");
    // else if (opponent->role == PLAY) printf("Player's turn.\n");
    // max game turns
    if (game->current_turn >= 50) 
    {
        game->game_over = true;
        game->winner = TIE;
    }
    if (player->active_pokemon == NULL)
    {
        game->game_over = true;
        game->winner = opponent->role;
    } else if (opponent->active_pokemon == NULL && opponent->bench_count == 0 && game->current_turn != 0)
    {
        game->game_over = true;
        game->winner = player->role;
    }
    // reset card stuff 
    reset_just_played(player);
    if (game->game_over) return true;
    return false;
}

Role get_winner(GameState *game)
{
    return game->winner;
}

Player * get_current_player_(GameState *game)
{
    Player *current_player = (game->current_turn % 2 == 0) ? &game->player1 : &game->player2;
    if (game->current_turn == 0)
    {
        if (game->current_player == PLAY) current_player = &game->player1;
        else  current_player = &game->player2;
    }
    if (game->turn_effects.sabrina_switch)
    {
        // other player is required to switch
        current_player = (game->current_turn % 2 == 0) ? &game->player2 : &game->player1;;
    }
    
    return current_player;
}

Player * get_opponent_(GameState *game)
{
    Player *opponent = (game->current_turn % 2 == 0) ? &game->player2 : &game->player1;
    if (game->current_turn == 0)
    {
        if (game->current_player == PLAY) opponent = &game->player2;
        else  opponent = &game->player1;
    }
    if (game->turn_effects.sabrina_switch)
    {
        // other player is required to switch
        opponent = (game->current_turn % 2 == 0) ? &game->player1 : &game->player2 ;
    }
    return opponent;
}

void print_player_state(Player *player, const char *player_name) {
    if (GAME_STATE_DEBUG) printf("%s:\n", player_name);
    
    if (GAME_STATE_DEBUG) printf("Hand: ");
    for (int i = 0; i < player->hand_count; i++) {
        if (GAME_STATE_DEBUG) printf("%s", player->hand[i]->name);
        if (i < player->hand_count - 1 && GAME_STATE_DEBUG) printf(", ");
    }
    if (GAME_STATE_DEBUG) printf("\n");
    
    if (GAME_STATE_DEBUG) printf("Discard pile: ");
    for (int i = 0; i < player->discard_count; i++) {
        if (GAME_STATE_DEBUG) printf("%s", player->discard_pile[i]->name);
        if (i < player->discard_count - 1 && GAME_STATE_DEBUG) printf(", ");
    }
    if (GAME_STATE_DEBUG) printf("\n");
    
    if (GAME_STATE_DEBUG) printf("Cards in deck: %d\n", player->deck->card_count);
    
    if (GAME_STATE_DEBUG) printf("Active Pokemon: ");
    if (player->active_pokemon) {
        if (GAME_STATE_DEBUG) printf("%s (HP: %d/%d) - Energies: ", player->active_pokemon->name, player->active_pokemon->hp, player->active_pokemon->hp_total);
        
        int energy_count = 0;
        for (int i = 0; i < MAX_CARD_ENERGIES; i++) {
            if (player->active_pokemon->attached_energies[i] > 0) {
                if (GAME_STATE_DEBUG) printf("%s %d ", energy_type_to_string((EnergyType)i), player->active_pokemon->attached_energies[i]);
                energy_count++;
            }
        }
        
        if (energy_count == 0) {
            if (GAME_STATE_DEBUG) printf("None");
        }
        if (GAME_STATE_DEBUG) printf("\n");
    } else {
        if (GAME_STATE_DEBUG) printf("None\n");
    }

    
    if (GAME_STATE_DEBUG) printf("Bench: ");
    for (int i = 0; i < player->bench_count; i++) {
        if (GAME_STATE_DEBUG) printf("%s (HP: %d/%d) - Energies: ", player->bench[i]->name, player->bench[i]->hp, player->bench[i]->hp_total);
        
        int energy_count = 0;
        for (int j = 0; j < MAX_CARD_ENERGIES; j++) {
            if (player->bench[i]->attached_energies[j] > 0) {
                if (GAME_STATE_DEBUG) printf("%s %d ", energy_type_to_string((EnergyType)j), player->bench[i]->attached_energies[j]);
                energy_count++;
            }
        }
        
        if (energy_count == 0) {
            if (GAME_STATE_DEBUG) printf("None");
        }
        
        if (i < player->bench_count - 1 && GAME_STATE_DEBUG) printf(", ");
    }
    if (GAME_STATE_DEBUG) printf("\n");

    if (GAME_STATE_DEBUG) printf("\n\n");
    if (GAME_STATE_DEBUG) fflush(stdout);
}
