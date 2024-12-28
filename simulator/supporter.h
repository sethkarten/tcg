#ifndef SUPPORTER_H
#define SUPPORTER_H

#include "game_state.h"
#include "player.h"

void erika_effect(Player *player, char *target);
void misty_effect(Player *player);
void blaine_effect(GameState *game);
void koga_effect(Player *player);
void giovanni_effect(GameState *game);
void brock_effect(Player *player);
void sabrina_effect(GameState *game, Player *opponent);
void lt_surge_effect(Player *player);
void budding_expeditioner_effect(Player *player);
void blue_effect(GameState *game);
void leaf_effect(GameState *game);
void professors_research_effect(GameState *game);

bool play_supporter(GameState *game, Player *player, char *card_name, int target, bool *supporter_played);

#endif // SUPPORTER_H
