#pragma once
#include <stdio.h>
#include <stdlib.h>

/*Модель хранения
хранит:
    указатель на буффер
    количество строк
    длину буффера
    максимальную длину строки
    указатели на начала исходных строк
*/
typedef struct Storage {
    char* text;
    int amount, len, maxStrLen;
    char** strPtr;
} Storage;

/*Заполняет модель хранения
in: FILE*
in/out: Storage*
out: int success code
*/
int fillStorage(Storage* storage, FILE* fp);

/*Вычисляет максимальную длину строки
in: Storage*
out: max string length
*/
int maxStrLen(Storage* storage);
