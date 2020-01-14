#include "Storage.h"

int fillStorage(Storage* storage, FILE* fp) {
    storage->amount = 0;
    storage->len = 0;

    fseek(fp,0,SEEK_END);
    storage->len = ftell(fp);

	if (storage->len == 0) {
		storage->text = (char*)calloc(1, sizeof(char));
		storage->text[0] = "\0";
		storage->strPtr = (char**)calloc(1, sizeof(char*));
		storage->strPtr[0] = storage->text;
		storage->amount = 1;
		storage->len = 1;
		storage->maxStrLen = 1;
		return 2;
	}
	else {

		//printf("%i\n", storage->len);

		storage->text = (char*)calloc(storage->len, sizeof(char) + 1);

		if (!storage->text) {
			return 1;
		}

		fseek(fp, 0, SEEK_SET);

		if (fread(storage->text, sizeof(char), storage->len, fp) != storage->len) {
			return 1;
		}
	}

    for (int  i = 0; i < storage->len; i++) {
        if (storage->text[i] == '\n') storage->amount++;
    }
    if (storage->text[storage->len - 1] != '\n') storage->amount++;

    storage->strPtr = (char**) calloc(storage->amount + 1, sizeof(char*));

    if (!storage->strPtr) {
        return 1;
    }

    char* bufPtr = storage->text;
    int j = 0;
    for (int  i = 0; j < storage->amount, i < storage->len; i++) {
        if (storage->text[i] == '\n') {
            storage->strPtr[j] = bufPtr;
            if (i == storage->len - 1) break;
            bufPtr = storage->text + i + 1;
            j++;
        }
    }
    if (storage->text[storage->len - 1] != '\n') storage->strPtr[j] = bufPtr;

    storage->maxStrLen = maxStrLen(storage);

    return 0;
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
