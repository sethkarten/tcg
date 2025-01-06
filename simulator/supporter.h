#ifndef SUPPORTER_H
#define SUPPORTER_H

#include "game_state.h"
#include "player.h"

bool erika_effect(Card *target);
bool misty_effect(Card *target);
bool blaine_effect(GameState *game);
bool koga_effect(Player *player);
bool giovanni_effect(GameState *game);
bool brock_effect(Card *card);
bool sabrina_effect(GameState *game, Player *opponent);
bool lt_surge_effect(Player *player);
bool budding_expeditioner_effect(Player *player);
bool blue_effect(GameState *game);
bool leaf_effect(GameState *game);
bool professors_research_effect(Player *player);

bool play_supporter(GameState *game, Player *player, char *card_name, int target);
bool supporter_is_activatable(GameState * game, Card *card);

#endif // SUPPORTER_H
