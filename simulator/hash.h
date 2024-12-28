#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "card.h"

// Linked List node
struct node {
    char* key;
    Card* value;
    struct node* next;
};

typedef struct {
    int numOfElements;
    int capacity;
    struct node** arr;
} HashMap;

// Function prototypes
void setNode(struct node* node, char* key, Card* value);
void initializeHashMap(HashMap* mp);
int hashFunction(HashMap* mp, char* key);
void insert(HashMap* mp, char* key, Card* value);
void delete(HashMap* mp, char* key);
Card* search(HashMap* mp, char* key);

#endif // HASH_H
