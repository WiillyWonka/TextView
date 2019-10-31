#include "Storage.h"

void fillStorage(Storage* storage, FILE* fp) {
    storage->amount = 0;
    storage->len = 0;

    fseek(fp,0,SEEK_END);
    storage->len = ftell(fp);

    //printf("%i\n", storage->len);

    storage->text = (char*) calloc(storage->len, sizeof(char)+1);

    if (!storage->text) {
        printf("Memory is not allocate");
        return;
    }

    fseek(fp,0,SEEK_SET);

    if (fread(storage->text, sizeof(char), storage->len, fp) != storage->len) {
        printf("fread != len");
    }

    for (int  i = 0; i < storage->len; i++) {
        if (storage->text[i] == '\n') storage->amount++;
    }

    storage->strPtr = (char**) calloc(storage->amount, sizeof(char*));

    //printf("Memory was allocated\n");

    char** bufPtr = storage->strPtr;
    *bufPtr = storage->text;
    bufPtr++;
    for (int  i = 0; i < storage->len; i++) {
        if (storage->text[i] == '\n') {
            *bufPtr = storage->text + i + 1;
            //printf(*bufPtr);
            bufPtr++;
        }
    }

    storage->maxStrLen = maxStrLen(storage);
    //printf(storage->text + storage->len - 100);
    //printf("\n");
}

int maxStrLen(Storage* storage) {
    int len;
    int max = 0;
    for (int i = 1; i < storage->amount; i++) {
        len = (int)(storage->strPtr[i] - storage->strPtr[i - 1]);
        //printf("%d", len);
        if (len > max) max = len;
    }
    //printf("%i", );
    return max - 1;
}
