#include "game_state.h"
#include "data.h"

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

void test_load_card_data() {
    GameState game;
    game.card_dictionary = (HashMap*)malloc(sizeof(HashMap));
    initializeHashMap(game.card_dictionary);

    load_card_data_from_json(&game, "pokemon_tcg_pocket_cards.json");

    printf("Loaded cards:\n");
    for (int i = 0; i < game.card_dictionary->capacity; i++) {
        struct node* current = game.card_dictionary->arr[i];
        while (current != NULL) {
            Card* card = (Card*)current->value;
            printf("Number: %s\n", card->number);
            printf("Name: %s\n", card->name);
            printf("Type: %s\n", energy_type_to_string(card->type));
            printf("HP: %d\n", card->hp);
            printf("Stage: %d\n", card->stage);
            printf("Retreat Cost: %d\n", card->retreat_cost);
            printf("Is EX: %s\n", card->is_ex ? "Yes" : "No");
            printf("Has Ability: %s\n", card->has_ability ? "Yes" : "No");
            printf("Moves:\n");
            for (int j = 0; j < card->move_count; j++) {
                printf("  - %s\n", card->moves[j].name);
                printf("    Energies: ");
                for (int k = 0; k < MAX_CARD_ENERGIES; k++) {
                    if (card->moves[j].energy[k] != 0)
                    {
                        printf("%s %d ", energy_type_to_string((EnergyType) k), card->moves[j].energy[k]);

                    }
                }
                printf("\n");
                printf("    Damage: %d\n", card->moves[j].damage);
            }
            printf("\n");
            current = current->next;
        }
    }


    // Clean up
    for (int i = 0; i < game.card_dictionary->capacity; i++) {
        struct node* current = game.card_dictionary->arr[i];
        while (current != NULL) {
            struct node* temp = current;
            current = current->next;
            free(temp->value);
            free(temp);
        }
    }
    free(game.card_dictionary->arr);
    free(game.card_dictionary);
}


int main() {
    test_load_card_data();
    return 0;
}
