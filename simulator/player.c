#include "player.h"
#include <stdlib.h>

void initialize_player(Player *player, Role role) {
    player->active_pokemon = NULL;
    player->bench_count = 0;
    player->hand_count = 0;
    player->prize_cards_left = 3;
    player->discard_count = 0;
    player->role = role;
}
