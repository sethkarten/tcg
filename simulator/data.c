#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

#include "data.h"
#include "card.h"
#include "hash.h"

#define DATA_DEBUG false

static void parse_move(cJSON *move_json, Move *move) {
    cJSON *name = cJSON_GetObjectItemCaseSensitive(move_json, "name");
    cJSON *energy = cJSON_GetObjectItemCaseSensitive(move_json, "energy");
    cJSON *damage = cJSON_GetObjectItemCaseSensitive(move_json, "damage");
    cJSON *description = cJSON_GetObjectItemCaseSensitive(move_json, "description");

    move->energy = NULL;
    move->description = NULL;
    move->name = NULL;
    move->energy_count = 0;

    if (cJSON_IsString(name) && name->valuestring != NULL) {
        move->name = strdup(name->valuestring);
    }

    if (cJSON_IsArray(energy)) {
        int energy_count = cJSON_GetArraySize(energy);
        move->energy = (int *)calloc(MAX_CARD_ENERGIES, sizeof(int));
        move->energy_count = (energy_count < MAX_CARD_ENERGIES) ? energy_count : MAX_CARD_ENERGIES;
        for (int i = 0; i < move->energy_count; i++) {
            cJSON *energy_item = cJSON_GetArrayItem(energy, i);
            if (cJSON_IsString(energy_item) && energy_item->valuestring != NULL) {
                move->energy[(int)parse_energy_type(energy_item->valuestring)] += 1;
            }
        }
    }

    if (cJSON_IsString(damage) && damage->valuestring != NULL) {
        move->damage = atoi(damage->valuestring);
    } else {
        move->damage = 0;
    }

    if (cJSON_IsString(description) && description->valuestring != NULL) {
        move->description = strdup(description->valuestring);
    }
}


// Function to check if a string contains another string (case-insensitive)
bool str_contains_insensitive(const char *str, const char *substr) {
    if (DATA_DEBUG) printf("Comparing %s vs. %s\n", str, substr);
    char *str_lower = strdup(str);
    char *substr_lower = strdup(substr);
    for (char *p = str_lower; *p; ++p) *p = tolower(*p);
    for (char *p = substr_lower; *p; ++p) *p = tolower(*p);
    
    bool result = strstr(str_lower, substr_lower) != NULL;
    
    if (DATA_DEBUG) printf("freeing data.c:L63\n");
    if (str_lower) free(str_lower);
    if (substr_lower) free(substr_lower);
    return result;
}


void load_card_data_from_json(GameState *game, const char* filename) {
    if (DATA_DEBUG) printf("Starting to load card data from file: %s\n", filename);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }
    if (DATA_DEBUG) printf("File opened successfully\n");

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (DATA_DEBUG) printf("File size: %ld bytes\n", file_size);

    char* json_string = (char*)malloc(file_size + 1);
    if (json_string == NULL) {
        fprintf(stderr, "Memory allocation failed for json_string\n");
        fclose(file);
        return;
    }
    if (DATA_DEBUG) printf("Memory allocated for json_string\n");

    fread(json_string, 1, file_size, file);
    fclose(file);
    json_string[file_size] = '\0';
    if (DATA_DEBUG) printf("JSON string read from file\n");

    cJSON* json = cJSON_Parse(json_string);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "JSON parsing error: %s\n", error_ptr);
        }
        if (DATA_DEBUG) printf("freeing data.c:L104\n");
        free(json_string);
        return;
    }
    if (DATA_DEBUG) printf("JSON parsed successfully\n");

    initializeHashMap(game->card_dictionary);
    if (DATA_DEBUG) printf("HashMap initialized\n");

    cJSON* card_item = NULL;
    int card_count = 0;
    cJSON_ArrayForEach(card_item, json) {  
        if (DATA_DEBUG) printf("Processing card %d\n", card_count);
        Card* card = (Card*)malloc(sizeof(Card));
        if (card == NULL) {
            fprintf(stderr, "Memory allocation failed for card\n");
            continue;
        }
        // memset(card, 0, sizeof(Card));

        // Dynamically allocate memory for card fields
        card->number = NULL;
        card->name = NULL;
        card->evolves_from = NULL;
        card->moves = NULL;
        card->attached_energies = (int *)calloc(MAX_CARD_ENERGIES, sizeof(int));
        init_card(card);
        card->id = card_count++;
        
        cJSON* card_number = cJSON_GetObjectItemCaseSensitive(card_item, "#");
        cJSON* card_name = cJSON_GetObjectItemCaseSensitive(card_item, "card_name");
        cJSON* type = cJSON_GetObjectItemCaseSensitive(card_item, "type");
        cJSON* weakness = cJSON_GetObjectItemCaseSensitive(card_item, "weakness");
        cJSON* hp = cJSON_GetObjectItemCaseSensitive(card_item, "hp");
        cJSON* stage = cJSON_GetObjectItemCaseSensitive(card_item, "stage");
        cJSON* retreat_cost = cJSON_GetObjectItemCaseSensitive(card_item, "retreat_cost");
        cJSON* ability = cJSON_GetObjectItemCaseSensitive(card_item, "ability");
        cJSON* moves = cJSON_GetObjectItemCaseSensitive(card_item, "moves");
        cJSON *evolves_from = cJSON_GetObjectItemCaseSensitive(card_item, "evolves_from");

        if (cJSON_IsString(evolves_from)) {
            card->evolves_from = strdup(evolves_from->valuestring);
        }

        if (DATA_DEBUG) printf("Card fields extracted\n");

        if (cJSON_IsString(card_number) && card_number->valuestring != NULL) {
            card->number = strdup(card_number->valuestring);
            if (DATA_DEBUG) printf("Card number: %s\n", card->number);
        }
        if (cJSON_IsString(card_name) && card_name->valuestring != NULL) {
            card->name = strdup(card_name->valuestring);
            if (DATA_DEBUG) printf("Card name: %s\n", card->name);
        } else printf("ERROR card name\n");

        if (cJSON_IsString(type) && type->valuestring != NULL) {
            const char* type_str = type->valuestring;
            card->cardtype = POKEMON;
            if (strcmp(type_str, "Grass") == 0) card->type = GRASS;
            else if (strcmp(type_str, "Fire") == 0) card->type = FIRE;
            else if (strcmp(type_str, "Water") == 0) card->type = WATER;
            else if (strcmp(type_str, "Lightning") == 0) card->type = LIGHTNING;
            else if (strcmp(type_str, "Psychic") == 0) card->type = PSYCHIC;
            else if (strcmp(type_str, "Fighting") == 0) card->type = FIGHTING;
            else if (strcmp(type_str, "Darkness") == 0) card->type = DARKNESS;
            else if (strcmp(type_str, "Metal") == 0) card->type = METAL;
            else if (strcmp(type_str, "Fairy") == 0) card->type = FAIRY;
            else if (strcmp(type_str, "Dragon") == 0) card->type = DRAGON;
            else if (strcmp(type_str, "Colorless") == 0) card->type = COLORLESS;
            else {
                if (strcmp(type_str, "Item") == 0) card->cardtype = ITEM;
                else if (strcmp(type_str, "Supporter") == 0) card->cardtype = SUPPORTER;
            }
            if (DATA_DEBUG) printf("Card type: %s\n", type_str);
        }

        if (cJSON_IsString(weakness) && weakness->valuestring != NULL) {
            const char* weakness_str = weakness->valuestring;
            if (DATA_DEBUG) printf("Card weakness: %s\n", weakness_str);
            if (strcmp(weakness_str, "Grass") == 0) card->weakness = GRASS;
            else if (strcmp(weakness_str, "Fire") == 0) card->weakness = FIRE;
            else if (strcmp(weakness_str, "Water") == 0) card->weakness = WATER;
            else if (strcmp(weakness_str, "Lightning") == 0) card->weakness = LIGHTNING;
            else if (strcmp(weakness_str, "Psychic") == 0) card->weakness = PSYCHIC;
            else if (strcmp(weakness_str, "Fighting") == 0) card->weakness = FIGHTING;
            else if (strcmp(weakness_str, "Darkness") == 0) card->weakness = DARKNESS;
            else if (strcmp(weakness_str, "Metal") == 0) card->weakness = METAL;
            else if (strcmp(weakness_str, "Fairy") == 0) card->weakness = FAIRY;
            else if (strcmp(weakness_str, "Dragon") == 0) card->weakness = DRAGON;
            else if (strcmp(weakness_str, "Colorless") == 0) card->weakness = COLORLESS;
            else card->weakness = NONE_TYPE;
        }

        if (cJSON_IsNumber(hp)) {
            card->hp = hp->valueint;
            card->hp_total = card->hp;
        }
        else if (cJSON_IsString(hp)) {
            card->hp = atoi(hp->valuestring);
            card->hp_total = card->hp;
        }
        if (DATA_DEBUG) printf("Card HP: %d\n", card->hp);

        if (cJSON_IsString(stage) && stage->valuestring != NULL) {
            if (strcmp(stage->valuestring, "Basic") == 0) card->stage = BASIC;
            else if (strcmp(stage->valuestring, "Stage 1") == 0) card->stage = STAGE1;
            else if (strcmp(stage->valuestring, "Stage 2") == 0) card->stage = STAGE2;
            if (DATA_DEBUG) printf("Card stage: %s\n", stage->valuestring);
        }

        if (cJSON_IsNumber(retreat_cost)) {
            card->retreat_cost = retreat_cost->valueint;
        }
        else if (cJSON_IsString(retreat_cost)) {
            card->retreat_cost = atoi(retreat_cost->valuestring);
        }
        if (DATA_DEBUG) printf("Card retreat cost: %d\n", card->retreat_cost);

        // card->is_ex = str_contains_insensitive(card->name, "ex");
        card->is_ex = strstr(card->name, "ex");
        if (DATA_DEBUG) printf("Card is EX: %s\n", card->is_ex ? "Yes" : "No");

        card->has_ability = (cJSON_IsArray(ability) && cJSON_GetArraySize(ability) > 0);
        if (DATA_DEBUG) printf("Card has ability: %s\n", card->has_ability ? "Yes" : "No");

        card->ability = (Ability*)malloc(sizeof(Ability));
        if (card->ability == NULL) {
            fprintf(stderr, "Memory allocation failed for ability\n");
        } else {
            card->ability->name = NULL;
            card->ability->description = NULL;

            if (cJSON_IsArray(ability)) {
                cJSON *ability_item = cJSON_GetArrayItem(ability, 0);
                cJSON *name = cJSON_GetObjectItemCaseSensitive(ability_item, "name");
                cJSON *description = cJSON_GetObjectItemCaseSensitive(ability_item, "description");

                if (cJSON_IsString(name) && name->valuestring != NULL) {
                    card->ability->name = strdup(name->valuestring);
                } else card->ability->name = strdup("");
                if (cJSON_IsString(description) && description->valuestring != NULL) {
                    card->ability->description = strdup(description->valuestring);
                } else card->ability->description = strdup("");
            }
        }



         if (cJSON_IsArray(moves)) {
            int move_count = cJSON_GetArraySize(moves);
            card->move_count = (move_count < MAX_MOVES) ? move_count : MAX_MOVES;
            card->moves = (Move *)malloc(card->move_count * sizeof(Move));
            for (int i = 0; i < card->move_count; i++) {
                cJSON *move_item = cJSON_GetArrayItem(moves, i);
                parse_move(move_item, &card->moves[i]);
            }
        }

        if (DATA_DEBUG) printf("Inserting card into HashMap\n");
        insert(game->card_dictionary, card->number, card);
        if (DATA_DEBUG) printf("Card inserted successfully\n");
    }

    if (DATA_DEBUG) printf("All cards processed. Total cards: %d\n", card_count);

    cJSON_Delete(json);
    if (DATA_DEBUG) printf("freeing data.c:L268\n");
    free(json_string);
    if (DATA_DEBUG) printf("Memory cleaned up\n");
}

