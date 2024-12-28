#include <string.h>
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
    return COLORLESS;  // Default type
}

void initialize_card(Card* card, const char* name, EnergyType type, int hp, Stage stage, int retreat_cost, bool is_ex, bool has_ability) {
    strncpy(card->name, name, MAX_CARD_NAME_LENGTH - 1);
    card->name[MAX_CARD_NAME_LENGTH - 1] = '\0';
    card->type = type;
    card->hp = hp;
    card->stage = stage;
    card->retreat_cost = retreat_cost;
    card->is_ex = is_ex;
    card->has_ability = has_ability;
    card->move_count = 0;
    card->energies_count = 0;
    for (int i = 0; i < MAX_CARD_ENERGIES; i++) card->attached_energies[i] = 0;
}

void attach_energy_to_card(Card* card, EnergyType energy) {
    if (card->energies_count < MAX_CARD_ENERGIES) {
        card->attached_energies[card->energies_count] = energy;
        card->energies_count++;
    }
}
