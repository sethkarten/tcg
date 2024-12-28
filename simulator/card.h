#ifndef CARD_H
#define CARD_H

#include <stdbool.h>

#define MAX_MOVES 4
#define MAX_CARD_ENERGIES 11
#define MAX_CARD_NAME_LENGTH 50     // Name of card

typedef enum {
    GRASS, FIRE, WATER, LIGHTNING, PSYCHIC, FIGHTING, DARKNESS, METAL, FAIRY, DRAGON, COLORLESS
} EnergyType;

typedef enum {
    BASIC, STAGE1, STAGE2
} Stage;

typedef enum {
    POKEMON, SUPPORTER, ITEM
} CardType;

typedef struct {
    char name[50];
    int energy[MAX_CARD_ENERGIES];
    int energy_count;
    int damage;
} Move;

typedef struct {
    char number[MAX_CARD_NAME_LENGTH];
    char name[MAX_CARD_NAME_LENGTH];
    EnergyType type;
    int hp;
    Stage stage;
    int retreat_cost;
    Move moves[MAX_MOVES];
    int move_count;
    bool is_ex;
    int attached_energies[MAX_CARD_ENERGIES];
    int energies_count;
    bool has_ability;
    CardType cardtype;
} Card;

// Function prototypes
EnergyType parse_energy_type(const char* type_string);
void initialize_card(Card* card, const char* name, EnergyType type, int hp, Stage stage, int retreat_cost, bool is_ex, bool has_ability);
void attach_energy_to_card(Card* card, EnergyType energy);

#endif // CARD_H
