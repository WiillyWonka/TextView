#include "PresentModel.h"
#include "menu.h"

void initPresentModel(HDC* hdc, PresentModel* presModel, LPARAM* lParam) {
    TEXTMETRIC tm;

    GetTextMetrics(*hdc, &tm);

    presModel->cxClient = LOWORD(*lParam);
    presModel->cyClient = HIWORD(*lParam);
    presModel->cxChar = tm.tmAveCharWidth;
    presModel->cyChar = tm.tmHeight + tm.tmExternalLeading;
    presModel->layoutMode = IDM_DEFAULT;
    presModel->storage = (Storage*)malloc(sizeof(Storage));
    presModel->iVscrollPos = presModel->iHscrollPos = 0;
    presModel->VCoef = presModel->VCoef = 1;
    presModel->strPtr = 0;
    presModel->amount = 0;
}

void fillPresentModel(PresentModel* presModel, FILE* fp) {
    fillStorage(presModel->storage, fp);

    presModel->amount = presModel->storage->amount;
    presModel->strPtr = presModel->storage->strPtr;
}

void PrintText(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel) {
    int a = presModel->startLine + ps->rcPaint.top / presModel->cyChar - 1;
    int iPaintBeg =  max(0, a);
    a = presModel->startLine + ps->rcPaint.bottom / presModel->cyChar;
    int iPaintEnd = min(presModel->storage->amount, a);
    int i, x, y, strLength;

    for(i = iPaintBeg; i < iPaintEnd; i++)
    {

        x = presModel->cxChar *(1 - presModel->iHscrollPos);
        y = presModel->cyChar *(1 - presModel->startLine + i);

        strLength = (i < presModel->amount - 1)?
                  presModel->strPtr[i + 1] - presModel->strPtr[i]:
                  presModel->storage->text + presModel->storage->len - presModel->strPtr[i];

        TextOut(*hdc, x, y,
                 presModel->strPtr[i],
                 strLength);
    }
}

void changeMode(PresentModel* presModel) {
    if (presModel->layoutMode) {
        presModel->layoutMode = 0;
        free(presModel->strPtr);
        presModel->strPtr = presModel->storage->strPtr;
        presModel->amount = presModel->storage->amount;
    } else {
        presModel->layoutMode = 1;
        reconfigureText(presModel);
    }
}

void reconfigureText(PresentModel* presModel) {
    int newAmount = 0;
    int strLen = 0;
    int i, j, xSize;

    xSize = presModel->cxClient / presModel->cxChar - 1;
    for (i = 1; i < presModel->storage->amount; i++) {
        strLen = (int)(presModel->storage->strPtr[i] - presModel->storage->strPtr[i - 1]);
        newAmount += strLen / xSize + 1;
    }

    char** strings = (char**)calloc(newAmount, sizeof(char*));

    char* bufPtr = 0;
    int delta;
    int newLine = presModel->startLine;
    for (i = 1, j = 0; i < presModel->storage->amount; i++) {
        bufPtr = presModel->storage->strPtr[i - 1];
        strLen = (int)(presModel->storage->strPtr[i] - presModel->storage->strPtr[i - 1]);
        strings[j] = bufPtr;

        if (presModel->cxClient - presModel->cxClientPrev > 0) {
            delta = strings[j] - presModel->strPtr[presModel->startLine];
            if (delta <= xSize && delta >= 0) {

                        newLine = j;
                        printf("new line %i\n", newLine);
            }
        }
        else {
            delta = presModel->strPtr[presModel->startLine] - strings[j];
            if (delta <= xSize && delta >= 0) {

                        newLine = j;
                        printf("new line %i\n", newLine);
            }
        }
        j++;
        while(presModel->storage->strPtr[i] - bufPtr > xSize) {
            int inc = xSize;
            strLen -= inc;
            bufPtr += inc;
            strings[j] = bufPtr;
            if (presModel->cxClient - presModel->cxClientPrev > 0) {
                delta = strings[j] - presModel->strPtr[presModel->startLine];
                if (delta <= xSize && delta >= 0) {
                            newLine = j;
                }
            }
            else {
                delta = presModel->strPtr[presModel->startLine] - strings[j];
                if (delta <= xSize && delta >= 0) {
                            newLine = j;
                }
            }
            j++;
        }
    }

    presModel->startLine = newLine;

    if(presModel->strPtr != presModel->storage->strPtr) {
        free(presModel->strPtr);
    }
    presModel->amount = newAmount;
    presModel->strPtr = strings;
}

void FreeModel(PresentModel* presModel) {
    if (presModel && presModel->storage) {
        if (presModel->storage->text) {
            free(presModel->storage->text);
        }
        free(presModel->storage);
    }
}

