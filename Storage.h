#pragma once
#include <stdio.h>
#include <stdlib.h>

/*������ ��������
������:
    ��������� �� ������
    ���������� �����
    ����� �������
    ������������ ����� ������
    ��������� �� ������ �������� �����
*/
typedef struct Storage {
    char* text;
    int amount, len, maxStrLen;
    char** strPtr;
} Storage;

/*��������� ������ ��������
in: FILE*
in/out: Storage*
out: int success code
*/
int fillStorage(Storage* storage, FILE* fp);

/*��������� ������������ ����� ������
in: Storage*
out: max string length
*/
int maxStrLen(Storage* storage);
