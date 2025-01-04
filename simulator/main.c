#include "game_state.h"
#include "data.h"

void test_load_card_data() {
    GameState game;
    game.card_dictionary = (HashMap*)malloc(sizeof(HashMap));
    initializeHashMap(game.card_dictionary);

    load_card_data_from_json(&game, "../pokemon_tcg_pocket_cards.json");

    printf("Loaded cards:\n");
    for (int i = 0; i < game.card_dictionary->capacity; i++) {
        struct node* current = game.card_dictionary->arr[i];
        while (current != NULL) {
            Card* card = (Card*)current->value;
            printf("Number: %s\n", card->number);
            printf("Name: %s\n", card->name);
            printf("Evolves From: %s\n", card->evolves_from);
            printf("Type: %s\n", energy_type_to_string(card->type));
            printf("Weakness: %s\n", energy_type_to_string(card->weakness));
            printf("HP: %d\n", card->hp);
            printf("HP Total: %d\n", card->hp_total);
            printf("Stage: %d\n", card->stage);
            printf("Retreat Cost: %d\n", card->retreat_cost);
            printf("Move Count: %d\n", card->move_count);
            printf("Moves:\n");
            for (int j = 0; j < card->move_count; j++) {
                printf("  - Name: %s\n", card->moves[j].name);
                printf("    Energies: ");
                for (int k = 0; k < MAX_CARD_ENERGIES; k++) {
                    if (card->moves[j].energy[k] != 0) {
                        printf("%s %d ", energy_type_to_string((EnergyType)k), card->moves[j].energy[k]);
                    }
                }
                printf("\n");
                printf("    Damage: %d\n", card->moves[j].damage);
                printf("    Description: %s\n", card->moves[j].description);
            }
            printf("Is EX: %s\n", card->is_ex ? "Yes" : "No");
            printf("Attached Energies: ");
            for (int k = 0; k < MAX_CARD_ENERGIES; k++) {
                if (card->attached_energies[k] != 0) {
                    printf("%s %d ", energy_type_to_string((EnergyType)k), card->attached_energies[k]);
                }
            }
            printf("\n");
            printf("Energies Count: %d\n", card->energies_count);
            printf("Has Ability: %s\n", card->has_ability ? "Yes" : "No");
            printf("Card Type: %d\n", card->cardtype);
            if (card->has_ability) {
                printf("Ability Name: %s\n", card->ability->name);
                printf("Ability Description: %s\n", card->ability->description);
            }
            printf("Status: %d\n", card->status);
            printf("Prevent Damage Next Turn: %s\n", card->prevent_damage_next_turn ? "Yes" : "No");
            printf("Ability Used: %s\n", card->ability_used ? "Yes" : "No");
            printf("\n");
            current = current->next;
        }
    }

    printf("Number of cards %d\n", game.card_dictionary->numOfElements);
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
