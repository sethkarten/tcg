#include <string.h>
#include <stdlib.h>
#include "card.h"

EnergyType parse_energy_type(const char* type_string) {
    if (strcmp(type_string, "Grass") == 0) return GRASS;
    if (strcmp(type_string, "Fire") == 0) return FIRE;
    if (strcmp(type_string, "Water") == 0) return WATER;
    if (strcmp(type_string, "Lightning") == 0) return LIGHTNING;
    if (strcmp(type_string, "Psychic") == 0) return PSYCHIC;
    if (strcmp(type_string, "Fighting") == 0) return FIGHTING;
    if (strcmp(type_string, "Darkness") == 0) return DARKNESS;
    if (strcmp(type_string, "Metal") == 0) return METAL;
    if (strcmp(type_string, "Fairy") == 0) return FAIRY;
    if (strcmp(type_string, "Dragon") == 0) return DRAGON;
    if (strcmp(type_string, "Colorless") == 0) return COLORLESS; // Default type
    return NONE_TYPE;  // No type
}

// this function is now deprecated
void initialize_card(Card* card, const char* name, EnergyType type, int hp, Stage stage, int retreat_cost, bool is_ex, bool has_ability) {
    strncpy(card->name, name, MAX_CARD_NAME_LENGTH - 1);
    card->name[MAX_CARD_NAME_LENGTH - 1] = '\0';
    // @todo: finish evolves from field
    card->evolves_from[MAX_CARD_NAME_LENGTH - 1] = '\0';
    card->type = type;
    card->hp = hp;
    card->stage = stage;
    card->retreat_cost = retreat_cost;
    card->is_ex = is_ex;
    card->has_ability = has_ability;
    init_card(card);
}

void init_card(Card * card)
{   
    card->move_count = 0;
    card->energies_count = 0;
    card->prevent_damage_next_turn = false;
    card->ability_used = false;
    for (int i = 0; i < MAX_CARD_ENERGIES; i++) card->attached_energies[i] = 0;
}

void attach_energy_to_card(Card* card, EnergyType energy) {
    card->attached_energies[(int) energy] += 1;
    card->energies_count++;
}


void discard_random_energy(Card *card) {
    if (card->energies_count == 0) {
        return;  // No energy to discard
    }

    // Find a random energy to discard
    int random_index;
    do {
        random_index = rand() % MAX_CARD_ENERGIES;
    } while (card->attached_energies[random_index] == 0);

    // Discard the energy
    card->attached_energies[random_index]--;
    card->energies_count--;

    return;
}


void heal_card(Card *card, int heal_amount) {
    if (card == NULL || card->cardtype != POKEMON) {
        return;  // Only heal Pokémon cards
    }

    card->hp += heal_amount;
    
    // Ensure HP doesn't exceed the total HP
    if (card->hp > card->hp_total) {
        card->hp = card->hp_total;
    }
}

// Card tokenizer function
int tokenize_card(Card* card) {
    // This function should return a unique integer for each card name
    // You would need to maintain a mapping of card names to integers
    // For simplicity, we'll use a hash function here
    return card->id;
    // int hash = 0;
    // for (int i = 0; card_name[i] != '\0'; i++) {
    //     hash = 31 * hash + card_name[i];
    // }
    // return (hash % MAX_CARDS) + 1; // Ensure non-zero token
}

const char* energy_type_to_string(EnergyType type) {
    switch (type) {
        case GRASS: return "Grass";
        case FIRE: return "Fire";
        case WATER: return "Water";
        case LIGHTNING: return "Lightning";
        case PSYCHIC: return "Psychic";
        case FIGHTING: return "Fighting";
        case DARKNESS: return "Darkness";
        case METAL: return "Metal";
        case FAIRY: return "Fairy";
        case DRAGON: return "Dragon";
        case COLORLESS: return "Colorless";
        default: return "Unknown";
    }
}