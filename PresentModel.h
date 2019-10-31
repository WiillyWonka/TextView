#pragma once

#include <windows.h>
#include "Storage.h"

typedef unsigned int uint;

typedef struct PresentModel {
    Storage* storage;
    int cxClient, cyClient, cxChar, cyChar, amount, iVscrollPos, iHscrollPos;
    double VCoef, HCoef;
    char** strPtr;
    int layoutMode;
} PresentModel;

void initPresentModel(HDC* hdc, PresentModel* presModel, LPARAM* lParam);
void fillPresentModel(PresentModel* presModel, FILE* fp);
void PrintText(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel);
//void LayoutMode(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel);
//void DefMode(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel);
void FreeModel(PresentModel* presModel);
void changeMode(PresentModel* presModel);
void reconfigureText(PresentModel* presModel);
