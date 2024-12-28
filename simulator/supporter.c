#include <string.h>
#include "supporter.h"
#include "card.h"
#include "deck.h"
#include "utils.h"

bool play_supporter(GameState *game, Player *player, char *card_name, int target, bool *supporter_played) {
    if (*supporter_played) {
        printf("You can only play one Supporter card per turn.\n");
        return false;
    }

    Card *card = find_card_in_hand(player, card_name);
    if (card == NULL || card->type != SUPPORTER) {
        printf("Invalid Supporter card.\n");
        return false;
    }

    *supporter_played = true;
    Player *opponent = &game->player1;
    if (player->role == PLAY) opponent = &game->player2;
    Card * target_card = get_target(player, opponent, target);

    bool valid_effect = true;

    if (strcmp(card_name, "Erika") == 0) valid_effect = erika_effect(target_card);
    else if (strcmp(card_name, "Misty") == 0) valid_effect = misty_effect(player);
    else if (strcmp(card_name, "Blaine") == 0) valid_effect = blaine_effect(game);
    else if (strcmp(card_name, "Koga") == 0) valid_effect = koga_effect(player);
    else if (strcmp(card_name, "Giovanni") == 0) valid_effect = giovanni_effect(game);
    else if (strcmp(card_name, "Brock") == 0) valid_effect = brock_effect(target_card);
    else if (strcmp(card_name, "Sabrina") == 0) valid_effect = sabrina_effect(game, opponent);
    else if (strcmp(card_name, "Lt. Surge") == 0) valid_effect = lt_surge_effect(player);
    else if (strcmp(card_name, "Budding Expeditioner") == 0) valid_effect = budding_expeditioner_effect(player);
    else if (strcmp(card_name, "Blue") == 0) valid_effect = blue_effect(game);
    else if (strcmp(card_name, "Leaf") == 0) valid_effect = leaf_effect(game);
    else if (strcmp(card_name, "Professor's Research") == 0) valid_effect = professors_research_effect(game);
    else {
        printf("Unknown Supporter card.\n");
        supporter_played = false;
        return;
    }

    discard_card_from_hand(player, card);
}

bool erika_effect(Card *target) {
    if (target) {
        if (target->type == GRASS) {
            heal_card(target, 50);
            printf("Healed 50 damage from %s\n", target->name);
            return true;
        }
    } 
    printf("No Grass Type Pokémon available\n");
    return false;
}

bool misty_effect(Player *player) {
    Card *water_pokemon = choose_water_pokemon(player);
    if (water_pokemon) {
        int heads = 0;
        while (flip_coin(1) == 1) {
            attach_energy_to_card(water_pokemon, WATER);
            heads++;
        }
        printf("Attached %d Water Energy to %s\n", heads, water_pokemon->name);
        return true;
    } else {
        printf("No Water Type Pokémon available\n");
        return false;
    }
}

bool blaine_effect(GameState *game) {
    game->turn_effects.blaine_boost = true;
    printf("Ninetales, Rapidash, and Magmar attacks do +30 damage this turn\n");
    return true;
}

bool koga_effect(Player *player) {
    Card *active = player->active_pokemon;
    if (strcmp(active->name, "Muk") == 0 || strcmp(active->name, "Weezing") == 0) {
        move_active_to_hand(player);
        printf("Moved %s to hand\n", active->name);
        return true;
    } else {
        printf("No Muk or Weezing in Active Spot\n");
        return false;
    }
}

bool giovanni_effect(GameState *game) {
    game->turn_effects.giovanni_boost = true;
    printf("All Pokémon attacks do +10 damage this turn\n");
    return true;
}

bool brock_effect(Card * target) {
    if (target) {
        if (strcmp(target->name, "Golem") == 0 || strcmp(target->name, "Onix") == 0) {
            attach_energy_to_card(target, FIGHTING);
            printf("Attached Fighting Energy to %s\n", target->name);
            return true;
        } 
    } 
    printf("No Golem or Onix available\n");
    return false;
}

bool sabrina_effect(GameState *game, Player *opponent) {
    if (opponent->bench_count > 0)
    {
        game->turn_effects.sabrina_switch = true;
        printf("Opponent's Active Pokémon switched to bench\n");
        return true;
    }
    printf("Invalid use of sabrina\n");
    return false;
}

bool lt_surge_effect(Player *player) {
    if (strcmp(player->active_pokemon->name, "Raichu") == 0 ||
        strcmp(player->active_pokemon->name, "Electrode") == 0 ||
        strcmp(player->active_pokemon->name, "Electabuzz") == 0) {
        move_lightning_energy_to_active(player);
        printf("Moved benched Lightning Energy to %s\n", player->active_pokemon->name);
        return true;
    } else {
        printf("No eligible Active Pokémon\n");
        return false;
    }
}

bool budding_expeditioner_effect(Player *player) {
    if (strcmp(player->active_pokemon->name, "Mew ex") == 0) {
         move_active_to_hand(player);
        printf("Moved Mew ex to hand\n");
        return true;
    } else {
        printf("No Mew ex in Active Spot\n");
        return false;
    }
}

bool blue_effect(GameState *game) {
    game->turn_effects.blue_protection = true;
    printf("All Pokémon take -10 damage next turn\n");
}

bool leaf_effect(GameState *game) {
    game->turn_effects.retreat_reduction += 2;
    printf("Active Pokémon Retreat Cost reduced by 2 this turn\n");
}

bool professors_research_effect(Player *player) {
    draw_card(&player->deck, player);
    draw_card(&player->deck, player);
    return true;
}

