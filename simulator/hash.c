#include "hash.h"

#define HASH_DEBUG false

void setNode(struct node* node, char* key, Card* value)
{
    node->key = key;
    node->value = value;
    node->next = NULL;
}

void initializeHashMap(HashMap* mp)
{
    if (HASH_DEBUG) printf("Entering initializeHashMap function\n");

    if (mp == NULL) {
        if (HASH_DEBUG) printf("Error: mp is NULL\n");
        return;
    }

    if (HASH_DEBUG) printf("Setting capacity\n");
    mp->capacity = 2048;

    if (HASH_DEBUG) printf("Setting numOfElements\n");
    mp->numOfElements = 0;

    if (HASH_DEBUG) printf("Allocating memory for arr\n");
    mp->arr = (struct node**)malloc(sizeof(struct node*) * mp->capacity);

    if (mp->arr == NULL) {
        if (HASH_DEBUG) printf("Error: Memory allocation failed for mp->arr\n");
        return;
    }

    if (HASH_DEBUG) printf("Initializing arr with memset\n");
    memset(mp->arr, 0, sizeof(struct node*) * mp->capacity);

    if (HASH_DEBUG) printf("Exiting initializeHashMap function successfully\n");
}


int hashFunction(HashMap* mp, char* key)
{
    int sum = 0, factor = 31;
    for (int i = 0; i < strlen(key); i++) {
        sum = ((sum % mp->capacity) + (((int)key[i]) * factor) % mp->capacity) % mp->capacity;
        factor = ((factor % __INT16_MAX__) * (31 % __INT16_MAX__)) % __INT16_MAX__;
    }
    return sum;
}

void insert(HashMap* mp, char* key, Card* value)
{
    if (HASH_DEBUG) printf("Entering insert function\n");
    if (HASH_DEBUG) printf("Key: %s\n", key);

    if (mp == NULL) {
        if (HASH_DEBUG) printf("Error: HashMap is NULL\n");
        return;
    }

    int bucketIndex = hashFunction(mp, key);
    if (HASH_DEBUG) printf("Bucket index: %d\n", bucketIndex);

    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    if (newNode == NULL) {
        if (HASH_DEBUG) printf("Error: Memory allocation failed for new node\n");
        return;
    }
    if (HASH_DEBUG) printf("New node allocated\n");

    setNode(newNode, key, value);
    if (HASH_DEBUG) printf("Node set with key and value\n");

    if (mp->arr[bucketIndex] == NULL) {
        if (HASH_DEBUG) printf("Bucket is empty, inserting new node\n");
        mp->arr[bucketIndex] = newNode;
    } else {
        if (HASH_DEBUG) printf("Bucket is not empty, inserting at the beginning of the list\n");
        newNode->next = mp->arr[bucketIndex];
        mp->arr[bucketIndex] = newNode;
    }

    mp->numOfElements++;
    if (HASH_DEBUG) printf("Number of elements increased to: %d\n", mp->numOfElements);

    if (HASH_DEBUG) printf("Exiting insert function\n");
}


void delete(HashMap* mp, char* key)
{
    int bucketIndex = hashFunction(mp, key);
    struct node* prevNode = NULL;
    struct node* currNode = mp->arr[bucketIndex];

    while (currNode != NULL) {
        if (strcmp(key, currNode->key) == 0) {
            if (currNode == mp->arr[bucketIndex]) {
                mp->arr[bucketIndex] = currNode->next;
            } else {
                prevNode->next = currNode->next;
            }
            // printf("freeing hash.c:L105\n");
            free(currNode);
            mp->numOfElements--;
            break;
        }
        prevNode = currNode;
        currNode = currNode->next;
    }
}

Card* search(HashMap* mp, char* key)
{
    int bucketIndex = hashFunction(mp, key);
    struct node* bucketHead = mp->arr[bucketIndex];
    while (bucketHead != NULL) {
        if (HASH_DEBUG) printf("search: %s -> %s\n", key, bucketHead->key);
        if (strcmp(bucketHead->key, key) == 0) {
            if (HASH_DEBUG) printf("Found.\n");
            return bucketHead->value;
        }
        bucketHead = bucketHead->next;
    }
    return NULL;
}
