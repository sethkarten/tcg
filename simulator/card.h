#ifndef CARD_H
#define CARD_H

#include <stdbool.h>

#define MAX_MOVES 4
#define MAX_CARD_ENERGIES 11
#define MAX_CARD_NAME_LENGTH 50     // Name of card
#define MAX_DESCRIPTION_LENGTH 128     // Name of card

typedef enum {
    GRASS, FIRE, WATER, LIGHTNING, PSYCHIC, FIGHTING, DARKNESS, METAL, FAIRY, DRAGON, COLORLESS, NONE_TYPE
} EnergyType;

typedef enum {
    BASIC, STAGE1, STAGE2
} Stage;

typedef enum {
    POKEMON, SUPPORTER, ITEM
} CardType;

typedef enum {
    NONE_STATUS, ASLEEP, PARALYZED, POISONED
} Status;

typedef struct {
    char *description;
    char *name;
} Ability;


typedef struct {
    char *description;
    char *name;
    int *energy;
    int energy_count;
    int damage;
} Move;


typedef struct {
    char *number;
    char *name;
    char *evolves_from;
    EnergyType type;
    EnergyType weakness;
    int hp;
    int hp_total;
    Stage stage;
    int retreat_cost;
    Move *moves;
    int move_count;
    bool is_ex;
    int *attached_energies;
    int energies_count;
    bool has_ability;
    CardType cardtype;
    Ability *ability;
    Status status;
    bool prevent_damage_next_turn;
    bool ability_used;
    int id;
} Card;


// Function prototypes
EnergyType parse_energy_type(const char* type_string);
void initialize_card(Card* card, const char* name, EnergyType type, int hp, Stage stage, int retreat_cost, bool is_ex, bool has_ability);
void init_card(Card * card);
void attach_energy_to_card(Card* card, EnergyType energy);
void discard_random_energy(Card *card);
void heal_card(Card *card, int heal_amount);
int tokenize_card(Card* card);
const char* energy_type_to_string(EnergyType type);

#endif // CARD_H
