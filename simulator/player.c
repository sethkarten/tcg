#include "player.h"

void initialize_player(Player *player) {
    player->bench_count = 0;
    player->hand_count = 0;
    player->prize_cards_left = 3;
    player->discount_count = 0;
}
