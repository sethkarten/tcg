#ifndef DATA_H
#define DATA_H

#include "card.h"
#include "game_state.h"

// Function to load card data from JSON file
void load_card_data_from_json(GameState *game, const char* filename);

#endif // DATA_H
