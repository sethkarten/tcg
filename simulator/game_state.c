#include "game_state.h"
#include "data.h"

void initialize_game_state(GameState *game, 
                            const char deck1[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH], 
                            bool energy1[MAX_CARD_ENERGIES], 
                            const char deck2[MAX_CARDS_IN_DECK][MAX_CARD_NAME_LENGTH],
                            bool energy2[MAX_CARD_ENERGIES]
) {
    // Load card data from JSON
    load_card_data_from_json(game, "pokemon_tcg_pocket_cards.json");

    // Initialize random seed
    srand(time(NULL));
    Role player1_role = PLAY;
    Role player2_role = OPP;

    // Init players
    initialize_player(&game->player1, player1_role);
    initialize_player(&game->player2, player2_role);

    // Initialize player 1's deck
    initialize_deck(game->card_dictionary, &game->player1.deck, deck1, energy1);

    // Initialize player 2's deck
    initialize_deck(game->card_dictionary, &game->player2.deck, deck2, energy2);

    // Shuffle decks
    shuffle_deck(&game->player1.deck);
    shuffle_deck(&game->player2.deck);

    // Initialize other game state variables
    game->current_turn = 0;
    game->supporter_played = false;
    game->game_over = false;
    game->turn_effects.blaine_boost = false;
    game->turn_effects.giovanni_boost = false;
    game->turn_effects.sabrina_switch = false;
    game->turn_effects.blue_protection = false;
    game->turn_effects.retreat_reduction = 0;

    // Draw initial hands
    draw_initial_hand(&game->player1.deck, game->player1);
    draw_initial_hand(&game->player2.deck, game->player2);

    // Determine who goes first by flipping a coin -- do this before initializing?
    
}

void start_turn(GameState *game, Player *player)
{
    // if not turn 1
    EnergyType energy = NULL;
    if (game->current_turn != 0)
    {
        draw_card(&player->deck, player);
        player->energy_available = true;
    }
    game->turn_effects.blaine_boost = false;
    game->turn_effects.giovanni_boost = false;
    game->turn_effects.sabrina_switch = false;
    game->turn_effects.retreat_reduction = 0;
}

bool act_turn(GameState *game, Player *player, char **action) 
{
    char *action_type = action[0];
    char *card_name = action[1];
    int target = (action[2] != NULL) ? atoi(action[2]) : -1;
    int turn_number = (int) game->current_turn / 2;

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
            return play_supporter(game, player, card_name, target, &game->supporter_played);
            break;
        case 'i':
            play_item(game, player, card_name, target);
            break;
        case 'p':
            play_pokemon(game, player, card_name);
            break;
        case 'v':
            evolve_pokemon(game, player, card_name, target);
            break;
        case 'r':
            retreat_pokemon(game, player, card_name, target);
            break;
        case 'a':
            use_ability(game, player, card_name, target);
            break;
        case 'm':
            use_move(game, player, card_name, target);
            end_turn(game);
            break;
        case 'e':
            end_turn(game);
            return;
        default:
            printf("Invalid action type: %c\n", action_type[0]);
    }
    return false;
}



bool play_item(GameState *game, Player *player, char *card_name, int target) {
    Card *card = search(player->hand, card_name);
    if (card == NULL || card->cardtype != ITEM) {
        return false;
        // Implement item card effect based on target
        // Remove card from hand and place in discard pile
    }

    if (strcmp(card_name, "Helix Fossil") == 0 ||
        strcmp(card_name, "Dome Fossil") == 0 ||
        strcmp(card_name, "Old Amber") == 0) {

    } else if (strcmp(card_name, "Pokemon Flute") == 0) {

    } else if (strcmp(card_name, "Mythical Slab") == 0) {

    } else if (strcmp(card_name, "Pokemon Flute") == 0) {

    } else if (strcmp(card_name, "Potion") == 0) {

    } else if (strcmp(card_name, "X Speed") == 0) {

    } else if (strcmp(card_name, "Pokedex") == 0) {

    } else if (strcmp(card_name, "Poke Ball") == 0) {

    } else if (strcmp(card_name, "Hand Scope") == 0) {

    } else if (strcmp(card_name, "Red Card") == 0) {

    } else {
        return false;
    }
}

void play_pokemon(GameState *game, Player *player, char *card_name) {
    Card *card = search(player->hand, card_name);
    if (card != NULL && card->type == POKEMON && card->stage == BASIC) {
        // Add Pokemon to bench if there's space
        // Remove card from hand
    }
}

void evolve_pokemon(GameState *game, Player *player, char *card_name, char *target) {
    Card *evolution = search(player->hand, card_name);
    Card *target_pokemon = search(player->field, target);
    if (evolution != NULL && target_pokemon != NULL && evolution->stage == target_pokemon->stage + 1) {
        // Evolve the target Pokemon
        // Remove evolution card from hand
    }
}

void retreat_pokemon(GameState *game, Player *player, char *card_name, char *target) {
    Card *active = search(player->field, card_name);
    Card *bench = search(player->bench, target);
    if (active != NULL && bench != NULL) {
        // Check if enough energy for retreat
        // Swap active and bench Pokemon
    }
}

void use_ability(GameState *game, Player *player, char *card_name, char *target) {
    Card *card = search(player->field, card_name);
    if (card != NULL && card->has_ability) {
        // Implement ability effect based on target
    }
}

void use_move(GameState *game, Player *player, char *card_name, char *target) {
    Card *card = search(player->field, card_name);
    if (card != NULL) {
        // Check if enough energy for move
        // Implement move effect based on target
    }
}

void end_turn(GameState *game) {
    game->current_turn++;
    game->supporter_played = false;
    // Switch active player
}

