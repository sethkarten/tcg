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

    // Init players
    initialize_player(&game->player1);
    initialize_player(&game->player2);

    // Initialize player 1's deck
    initialize_deck(game, &game->player1.deck, deck1);

    // Initialize player 2's deck
    initialize_deck(game, &game->player2.deck, deck2);

    // Shuffle decks
    shuffle_deck(&game->player1.deck);
    shuffle_deck(&game->player2.deck);

    // Initialize other game state variables
    game->current_turn = 0;
    game->game_over = false;

    // Draw initial hands
    draw_initial_hand(&game->player1.deck, player1);
    draw_initial_hand(&game->player2.deck, player2);

    // Determine who goes first by flipping a coin -- do this before initializing?

    
}

void act_turn(GameState *game, Player *player, char ***actions, int num_actions) 
{
    // if not turn 1
    EnergyType energy = NULL;
    if (game->current_turn != 0)
    {
        draw_card(&player->deck, player);
        energy = player->deck.energy_seq[(int) game->current_turn / 2];
    }

    // iterate through action list
    for (int i = 0; i < num_actions; i++) {
        char *action_type = actions[i][0];
        char *card_name = actions[i][1];
        char *target = (actions[i][2] != NULL) ? actions[i][2] : NULL;

        switch(action_type[0]) {
            case 's':
                play_supporter(game, player, card_name);
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
                break;
            case 'e':
                end_turn(game);
                return;
            default:
                printf("Invalid action type: %c\n", action_type[0]);
        }
    }
}

void play_supporter(GameState *game, Player *player, char *card_name) {
    Card *card = search(player->hand, card_name);
    if (card != NULL && card->type == SUPPORTER) {
        // Implement supporter card effect
        // Remove card from hand and place in discard pile
    }
}

void play_item(GameState *game, Player *player, char *card_name, char *target) {
    Card *card = search(player->hand, card_name);
    if (card != NULL && card->type == ITEM) {
        // Implement item card effect based on target
        // Remove card from hand and place in discard pile
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
    // Switch active player
}

