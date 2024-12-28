#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

#include "data.h"
#include "card.h"
#include "hash.h"

static void parse_move(cJSON *move_json, Move *move) {
    cJSON *name = cJSON_GetObjectItemCaseSensitive(move_json, "name");
    cJSON *energy = cJSON_GetObjectItemCaseSensitive(move_json, "energy");
    cJSON *damage = cJSON_GetObjectItemCaseSensitive(move_json, "damage");

    for (int i = 0; i < MAX_CARD_ENERGIES; i++) move->energy[i] = 0;

    if (cJSON_IsString(name) && name->valuestring != NULL) {
        strncpy(move->name, name->valuestring, sizeof(move->name) - 1);
        move->name[sizeof(move->name) - 1] = '\0';
    }

    if (cJSON_IsArray(energy)) {
        int energy_count = cJSON_GetArraySize(energy);
        move->energy_count = (energy_count < MAX_CARD_ENERGIES) ? energy_count : MAX_CARD_ENERGIES;
        for (int i = 0; i < move->energy_count; i++) {
            cJSON *energy_item = cJSON_GetArrayItem(energy, i);
            if (cJSON_IsString(energy_item) && energy_item->valuestring != NULL) {
                move->energy[(int) parse_energy_type(energy_item->valuestring)] += 1;
            }
        }
    }

    if (cJSON_IsString(damage) && damage->valuestring != NULL) {
        move->damage = atoi(damage->valuestring);
    }

}

// Function to check if a string contains another string (case-insensitive)
bool str_contains_insensitive(const char *str, const char *substr) {
    char *str_lower = strdup(str);
    char *substr_lower = strdup(substr);
    for (char *p = str_lower; *p; ++p) *p = tolower(*p);
    for (char *p = substr_lower; *p; ++p) *p = tolower(*p);
    
    bool result = strstr(str_lower, substr_lower) != NULL;
    
    free(str_lower);
    free(substr_lower);
    return result;
}


void load_card_data_from_json(GameState *game, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* json_string = (char*)malloc(file_size + 1);
    fread(json_string, 1, file_size, file);
    fclose(file);

    json_string[file_size] = '\0';

    cJSON* json = cJSON_Parse(json_string);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "JSON parsing error: %s\n", error_ptr);
        }
        free(json_string);
        return;
    }

    initializeHashMap(game->card_dictionary);
    cJSON* card_item = NULL;
    cJSON_ArrayForEach(card_item, json) {

        Card* card = (Card*)malloc(sizeof(Card));
        for (int i = 0; i < MAX_CARD_ENERGIES; i++) card->attached_energies[i] = 0;
        
        cJSON* card_number = cJSON_GetObjectItemCaseSensitive(card_item, "#");
        cJSON* card_name = cJSON_GetObjectItemCaseSensitive(card_item, "card_name");
        cJSON* type = cJSON_GetObjectItemCaseSensitive(card_item, "type");
        cJSON* hp = cJSON_GetObjectItemCaseSensitive(card_item, "hp");
        cJSON* stage = cJSON_GetObjectItemCaseSensitive(card_item, "stage");
        cJSON* retreat_cost = cJSON_GetObjectItemCaseSensitive(card_item, "retreat_cost");
        cJSON *ability = cJSON_GetObjectItemCaseSensitive(card_item, "ability");

        cJSON* moves = cJSON_GetObjectItemCaseSensitive(card_item, "moves");

        if (cJSON_IsString(card_number) && card_number->valuestring != NULL) {
            strncpy(card->number, card_number->valuestring, MAX_CARD_NAME_LENGTH - 1);
            card->number[MAX_CARD_NAME_LENGTH - 1] = '\0';
        }
        if (cJSON_IsString(card_name) && card_name->valuestring != NULL) {
            strncpy(card->name, card_name->valuestring, MAX_CARD_NAME_LENGTH - 1);
            card->name[MAX_CARD_NAME_LENGTH - 1] = '\0';
        }

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
        }


        if (cJSON_IsNumber(hp)) {
            card->hp = hp->valueint;
            card->hp_total = card->hp;
        }
        else if (cJSON_IsString(hp)) {
            card->hp = atoi(hp->valuestring);
            card->hp_total = card->hp;
        }

        if (cJSON_IsString(stage) && stage->valuestring != NULL) {
            if (strcmp(stage->valuestring, "Basic") == 0) card->stage = BASIC;
            else if (strcmp(stage->valuestring, "Stage 1") == 0) card->stage = STAGE1;
            else if (strcmp(stage->valuestring, "Stage 2") == 0) card->stage = STAGE2;
        }

        if (cJSON_IsNumber(retreat_cost)) {
            card->retreat_cost = retreat_cost->valueint;
        }
        else if (cJSON_IsString(retreat_cost)) {
            card->retreat_cost = atoi(retreat_cost->valuestring);
        }

        // In the card parsing section
        card->is_ex = str_contains_insensitive(card->name, "ex");

        card->has_ability = (cJSON_IsArray(ability) && cJSON_GetArraySize(ability) > 0);

        if (cJSON_IsArray(moves)) {
            int move_count = cJSON_GetArraySize(moves);
            card->move_count = (move_count < MAX_MOVES) ? move_count : MAX_MOVES;
            for (int i = 0; i < card->move_count; i++) {
                cJSON *move_item = cJSON_GetArrayItem(moves, i);
                parse_move(move_item, &card->moves[i]);
            }
        }

         insert(game->card_dictionary, card->number, card);
    }

    cJSON_Delete(json);
    free(json_string);
}
