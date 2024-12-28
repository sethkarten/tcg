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

