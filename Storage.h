#pragma once
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;

typedef struct Storage {
    char* text;
    int amount, len, maxStrLen;
    char** strPtr;
} Storage;

void fillStorage(Storage* storage, FILE* fp);

int maxStrLen(Storage* storage);
