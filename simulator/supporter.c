#include <string.h>
#include "supporter.h"
#include "card.h"
#include "deck.h"
#include "utils.h"
#include "player.h"

#define INFO_SUPPORTER false

bool play_supporter(GameState *game, Player *player, char *card_name, int target) {
    if (game->supporter_played) {
        fprintf(stdout, "You can only play one Supporter card per turn. %s\n", card_name);
        return false;
    }

    Card *card = find_card_in_hand(player, card_name);
    if (card == NULL || card->cardtype != SUPPORTER) {
        fprintf(stderr, "Invalid Supporter card: %s %d\n", card_name, target);
        return false;
    }

    Player *opponent = &game->player1;
    if (player->role == PLAY) opponent = &game->player2;
    if (opponent_has_shadowy_spellbind(opponent))
    {
        fprintf(stdout, "Opponent has shadowy spellbind\n");
        return false;
    }
    game->supporter_played = true;
    Card * target_card = get_target(player, opponent, target);

    bool valid_effect = true;

    if (strcmp(card_name, "Erika") == 0) valid_effect = erika_effect(target_card);
    else if (strcmp(card_name, "Misty") == 0) valid_effect = misty_effect(target_card);
    else if (strcmp(card_name, "Blaine") == 0) valid_effect = blaine_effect(game);
    else if (strcmp(card_name, "Koga") == 0) valid_effect = koga_effect(player);
    else if (strcmp(card_name, "Giovanni") == 0) valid_effect = giovanni_effect(game);
    else if (strcmp(card_name, "Brock") == 0) valid_effect = brock_effect(target_card);
    else if (strcmp(card_name, "Sabrina") == 0) valid_effect = sabrina_effect(game, opponent);
    else if (strcmp(card_name, "Lt. Surge") == 0) valid_effect = lt_surge_effect(player);
    else if (strcmp(card_name, "Budding Expeditioner") == 0) valid_effect = budding_expeditioner_effect(player);
    else if (strcmp(card_name, "Blue") == 0) valid_effect = blue_effect(game);
    else if (strcmp(card_name, "Leaf") == 0) valid_effect = leaf_effect(game);
    else if (strcmp(card_name, "Professor's Research") == 0) valid_effect = professors_research_effect(player);
    else {
        fprintf(stderr, "Unknown Supporter card.\n");
        return false;
    }
    if (valid_effect) discard_card_from_hand(player, card);
    return valid_effect;
}

bool supporter_is_activatable(GameState * game, Card *card)
{
    char * card_name = card->name;
    Player * player = get_current_player_(game);
    Player * opponent = get_opponent_(game);

    if (strcmp(card_name, "Erika") == 0) 
    {
        // Check for grass type with missing health
        if (player->active_pokemon && player->active_pokemon->type == GRASS && player->active_pokemon->hp < player->active_pokemon->hp_total)
            return true;
        for (int i = 0; i < player->bench_count; i++)
            if (player->bench[i]->type == GRASS && player->bench[i]->hp < player->bench[i]->hp_total)
                return true;
        return false;
    }
    else if (strcmp(card_name, "Misty") == 0) 
    {
        // Check for water type
        if (player->active_pokemon && player->active_pokemon->type == WATER)
            return true;
        for (int i = 0; i < player->bench_count; i++)
            if (player->bench[i]->type == WATER)
                return true;
        return false;
    }
    else if (strcmp(card_name, "Blaine") == 0) 
    {
        // Check for Ninetales, Rapidash, or Magmar
        if (player->active_pokemon && (strcmp(player->active_pokemon->name, "Ninetales") == 0 || 
            strcmp(player->active_pokemon->name, "Rapidash") == 0 || 
            strcmp(player->active_pokemon->name, "Magmar") == 0))
            return true;
        for (int i = 0; i < player->bench_count; i++)
            if (strcmp(player->bench[i]->name, "Ninetales") == 0 || 
                strcmp(player->bench[i]->name, "Rapidash") == 0 || 
                strcmp(player->bench[i]->name, "Magmar") == 0)
                return true;
        return false;
    }
    else if (strcmp(card_name, "Koga") == 0) 
    {
        // Check for Muk or Weezing
        if (player->active_pokemon && (strcmp(player->active_pokemon->name, "Muk") == 0 || 
            strcmp(player->active_pokemon->name, "Weezing") == 0))
            return true;
        return false;
    }
    else if (strcmp(card_name, "Giovanni") == 0) return true;
    else if (strcmp(card_name, "Brock") == 0) 
    {
        // Check for Golem or Onix
        if (player->active_pokemon && (strcmp(player->active_pokemon->name, "Golem") == 0 || 
            strcmp(player->active_pokemon->name, "Onix") == 0))
            return true;
        for (int i = 0; i < player->bench_count; i++)
            if (strcmp(player->bench[i]->name, "Golem") == 0 || 
                strcmp(player->bench[i]->name, "Onix") == 0)
                return true;
        return false;
    }
    else if (strcmp(card_name, "Sabrina") == 0) 
    {
        // Opponent must have benched pokemon
        return opponent->bench_count > 0;
    }
    else if (strcmp(card_name, "Lt. Surge") == 0) 
    {
        if (player->active_pokemon && (strcmp(player->active_pokemon->name, "Raichu") == 0 ||
            strcmp(player->active_pokemon->name, "Electrode") == 0 ||
            strcmp(player->active_pokemon->name, "Electabuzz") == 0))
        {
            return true;
        }
        return false;
    }
    else if (strcmp(card_name, "Budding Expeditioner") == 0) 
    {
        return player->active_pokemon && strcmp(player->active_pokemon->name, "Mew ex") == 0 && player->bench_count > 0;
    }
    else if (strcmp(card_name, "Blue") == 0) return true;
    else if (strcmp(card_name, "Leaf") == 0) return true;
    else if (strcmp(card_name, "Professor's Research") == 0) return true;
    
    // Default case: if the supporter card is not recognized, assume it's not activatable
    return false;
}


bool erika_effect(Card *target) {
    if (target) {
        if (target->type == GRASS) {
            heal_card(target, 50);
            if (INFO_SUPPORTER) printf("Healed 50 damage from %s\n", target->name);
            return true;
        }
    } 
    if (INFO_SUPPORTER) printf("No Grass Type Pokémon available\n");
    return false;
}

bool misty_effect(Card *target) {
    if (target) {
        if (target->type == WATER)
        {
            int heads = 0;
            while (flip_coin() == HEADS) {
                attach_energy_to_card(target, WATER);
                heads++;
            }
            if (INFO_SUPPORTER) printf("Attached %d Water Energy to %s\n", heads, target->name);
            return true;
        }
    } 
    if (INFO_SUPPORTER) printf("No Water Type Pokémon available\n");
    return false;
}

bool blaine_effect(GameState *game) {
    game->turn_effects.blaine_boost = true;
    if (INFO_SUPPORTER) printf("Ninetales, Rapidash, and Magmar attacks do +30 damage this turn\n");
    return true;
}

bool koga_effect(Player *player) {
    Card *active = player->active_pokemon;
    if (strcmp(active->name, "Muk") == 0 || strcmp(active->name, "Weezing") == 0) {
        move_active_to_hand(player);
        if (INFO_SUPPORTER) printf("Moved %s to hand\n", active->name);
        return true;
    } else {
        fprintf(stderr, "No Muk or Weezing in Active Spot\n");
        return false;
    }
}

bool giovanni_effect(GameState *game) {
    game->turn_effects.giovanni_boost = true;
    if (INFO_SUPPORTER) printf("All Pokémon attacks do +10 damage this turn\n");
    return true;
}

bool brock_effect(Card * target) {
    if (target) {
        if (strcmp(target->name, "Golem") == 0 || strcmp(target->name, "Onix") == 0) {
            attach_energy_to_card(target, FIGHTING);
            if (INFO_SUPPORTER) printf("Attached Fighting Energy to %s\n", target->name);
            return true;
        } 
    } 
    if (INFO_SUPPORTER) printf("No Golem or Onix available\n");
    return false;
}

bool sabrina_effect(GameState *game, Player *opponent) {
    if (opponent->bench_count > 0)
    {
        game->turn_effects.sabrina_switch = true;
        if (INFO_SUPPORTER) printf("Opponent's Active Pokémon switched to bench\n");
        return true;
    }
    if (INFO_SUPPORTER) printf("Invalid use of sabrina\n");
    return false;
}

bool lt_surge_effect(Player *player) {
    if (strcmp(player->active_pokemon->name, "Raichu") == 0 ||
        strcmp(player->active_pokemon->name, "Electrode") == 0 ||
        strcmp(player->active_pokemon->name, "Electabuzz") == 0) {
        move_lightning_energy_to_active(player);
        if (INFO_SUPPORTER) printf("Moved benched Lightning Energy to %s\n", player->active_pokemon->name);
        return true;
    } else {
        if (INFO_SUPPORTER) printf("No eligible Active Pokémon\n");
        return false;
    }
}

bool budding_expeditioner_effect(Player *player) {
    if (strcmp(player->active_pokemon->name, "Mew ex") == 0 && player->bench_count > 0) {
         move_active_to_hand(player);
        if (INFO_SUPPORTER) printf("Moved Mew ex to hand\n");
        return true;
    } else {
        if (INFO_SUPPORTER) printf("No Mew ex in Active Spot\n");
        return false;
    }
}

bool blue_effect(GameState *game) {
    game->turn_effects.blue_protection = true;
    if (INFO_SUPPORTER) printf("All Pokémon take -10 damage next turn\n");
    return true;
}

bool leaf_effect(GameState *game) {
    game->turn_effects.retreat_reduction += 2;
    if (INFO_SUPPORTER) printf("Active Pokémon Retreat Cost reduced by 2 this turn\n");
    return true;
}

bool professors_research_effect(Player *player) {
    if (player->deck->card_count > 0) draw_card(player, player->deck);
    if (player->deck->card_count > 0) draw_card(player, player->deck);
    return true;
}

