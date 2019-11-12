#include <limits.h>
#include "PresentModel.h"
#include "menu.h"

int initPresentModel(HDC* hdc, PresentModel* presModel, LPARAM* lParam,
                       FILE* fp, int mode)
{
    if (hdc) {
        TEXTMETRIC tm;

        GetTextMetrics(*hdc, &tm);
        presModel->cxChar = tm.tmAveCharWidth;
        presModel->cyChar = tm.tmHeight + tm.tmExternalLeading;
    }

    if (lParam) {
        presModel->cxClient = LOWORD(*lParam);
        presModel->cyClient = HIWORD(*lParam);
    }
    presModel->mode = mode;
    presModel->storage = (Storage*)malloc(sizeof(Storage));
    if (presModel->storage == 0) {
        return 1;
    }
    presModel->iVscrollPos = presModel->iHscrollPos = 0;
    presModel->VCoef = presModel->HCoef = 1;
    presModel->strPtr = 0;
    presModel->amount = 0;
    presModel->startLine = 0;

    if (fillStorage(presModel->storage, fp) != 0) {
        return 1;
    }

    presModel->amount = presModel->storage->amount;
    presModel->strPtr = presModel->storage->strPtr;

    if (presModel->mode == IDM_LAYOUT) {
        if (reconfigureText(presModel) != 0) return 1;
    }
    return 0;
}

void PrintText(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel) {
    if (presModel->strPtr == 0) return;
    int a = presModel->startLine + ps->rcPaint.top / presModel->cyChar;
    int iPaintBeg =  max(0, a);
    a = presModel->startLine + ps->rcPaint.bottom / presModel->cyChar;
    int iPaintEnd = min(presModel->amount, a);
    int i, x, y, strLength;

    for(i = iPaintBeg; i < iPaintEnd; i++)
    {

        x = presModel->cxChar *(1 - presModel->iHscrollPos);
        y = presModel->cyChar *(i - presModel->startLine);

        strLength = (i < presModel->amount - 1)?
                  presModel->strPtr[i + 1] - presModel->strPtr[i]:
                  presModel->storage->text + presModel->storage->len - presModel->strPtr[i];

        TextOut(*hdc, x, y,
                 presModel->strPtr[i],
                 strLength);
    }
}

int reconfigureText(PresentModel* presModel) {
    int newAmount = 0;
    int strLen = 0;
    int i, j, xSize;

    xSize = presModel->cxClient / presModel->cxChar;
    if (xSize == 0) {
        return 1;
    }
    for (i = 1; i < presModel->storage->amount; i++) {
        strLen = (int)(presModel->storage->strPtr[i] - presModel->storage->strPtr[i - 1]);
        newAmount += strLen / xSize + 1;
    }

    char** strings = (char**)calloc(newAmount, sizeof(char*));
    if (strings == 0) {
        return 1;
    }


    char* bufPtr = 0;
    char* startPtr = presModel->strPtr[presModel->startLine];
    int minLen = INT_MAX;
    int newLine = presModel->startLine;
    for (i = 1, j = 0; i < presModel->storage->amount; i++) {
        bufPtr = presModel->storage->strPtr[i - 1];
        if (abs(startPtr - bufPtr) < minLen) {
            minLen = abs(startPtr - bufPtr);
            presModel->startLine = j;
        }
        strLen = (int)(presModel->storage->strPtr[i] - presModel->storage->strPtr[i - 1]);
        strings[j] = bufPtr;
        j++;
        while(presModel->storage->strPtr[i] - bufPtr > xSize) {
            int inc = xSize;
            strLen -= inc;
            bufPtr += inc;
            strings[j] = bufPtr;
            j++;
        }
    }

    if(presModel->strPtr != presModel->storage->strPtr) {
        free(presModel->strPtr);
    }
    presModel->amount = j - 1;
    presModel->strPtr = strings;

    return 0;
}

void FreeModel(PresentModel* presModel) {
    if (presModel) {
        switch (presModel->mode) {
        case IDM_DEFAULT: {
            free(presModel->storage->text);
            free(presModel->strPtr);
            free(presModel->storage);
            break;
        }
        case IDM_LAYOUT:{
            free(presModel->strPtr);
            free(presModel->storage->text);
            free(presModel->storage);
            break;
        }
        }
    }
}

