#include "PresentModel.h"

void initPresentModel(HDC* hdc, PresentModel* presModel, LPARAM* lParam) {
    TEXTMETRIC tm;

    GetTextMetrics(*hdc, &tm);

    presModel->cxClient = LOWORD(*lParam);
    presModel->cyClient = HIWORD(*lParam);
    presModel->cxChar = tm.tmAveCharWidth;
    presModel->cyChar = tm.tmHeight + tm.tmExternalLeading;
    presModel->layoutMode = 1;
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
    double a = presModel->VCoef * presModel->iVscrollPos + ps->rcPaint.top / presModel->cyChar - 1;
    double iPaintBeg =  max(0, a);
    a = presModel->VCoef * presModel->iVscrollPos + ps->rcPaint.bottom / presModel->cyChar;
    double iPaintEnd = min(presModel->storage->amount, a);
    int HorzBeg = max(0, presModel->iHscrollPos + ps->rcPaint.left / presModel->cxChar);
    int rectLen = (ps->rcPaint.right - ps->rcPaint.left) / presModel->cxChar;
    int i, x, y, strLength;

    for(i = iPaintBeg; i < iPaintEnd; i++)
    {
        x = presModel->cxChar *(1 - presModel->iHscrollPos);
        y = presModel->cyChar *(1 - presModel->iVscrollPos*presModel->VCoef + i);

        strLength = (i < presModel->amount - 1)?
                  presModel->strPtr[i + 1] - presModel->strPtr[i]:
                  presModel->storage->text + presModel->storage->len - presModel->strPtr[i];

        TextOut(*hdc, x, y,
                 presModel->strPtr[i],
                 strLength);
    }


    /*double buf = presModel->VCoef * presModel->iVscrollPos + ps->rcPaint.top / presModel->cyChar - 1;
    double VertBeg =  max(0, buf);
    buf = presModel->VCoef * presModel->iVscrollPos + ps->rcPaint.bottom / presModel->cyChar;
    double VertEnd = min(presModel->amount, buf);

    buf = presModel->HCoef * presModel->iHscrollPos + ps->rcPaint.left / presModel->cxChar;
    double HorzBeg = max(0, buf);
    double HorzEnd = presModel->HCoef * presModel->iHscrollPos + ps->rcPaint.right / presModel->cxChar;
    double rectLen = (HorzEnd - HorzBeg);
    int i, x, y, strLength;

    printf("rectLen %f\n", rectLen);
    printf("cxClient/cxChar %d\n", presModel->cxClient/presModel->cxChar);
    for(i = VertBeg; i < VertEnd; i++)
    {
        x = presModel->cxChar *(1 - presModel->iHscrollPos*presModel->HCoef + HorzBeg);
        y = presModel->cyChar *(1 - presModel->iVscrollPos*presModel->VCoef + i);

        strLength = (i < presModel->storage->amount - 1)?
                  presModel->strPtr[i + 1] - presModel->strPtr[i]:
                  presModel->storage->text + presModel->storage->len - presModel->strPtr[i];

        buf = strLength - HorzBeg;
        if (buf > 0) {
            TextOut(*hdc, x, y,
                     presModel->strPtr[i] + rint(HorzBeg),
                     min(buf, (int)rectLen));
        }
    }*/
}
/*void LayoutMode(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel) {}
void DefMode(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel){
    double a = presModel->VCoef * presModel->iVscrollPos + ps->rcPaint.top / presModel->cyChar - 1;
    double iPaintBeg =  max(0, a);
    a = presModel->VCoef * presModel->iVscrollPos + ps->rcPaint.bottom / presModel->cyChar;
    double iPaintEnd = min(presModel->storage->amount, a);
    int HorzBeg = max(0, presModel->iHscrollPos + ps->rcPaint.left / presModel->cxChar);
    int rectLen = (ps->rcPaint.right - ps->rcPaint.left) / presModel->cxChar;
    int i, x, y, strLength;

    for(i = iPaintBeg; i < iPaintEnd; i++)
    {
        x = presModel->cxChar *(1 - presModel->iHscrollPos);
        y = presModel->cyChar *(1 - presModel->iVscrollPos*presModel->VCoef + i);

        strLength = (i < presModel->storage->amount - 1)?
                  presModel->strPtr[i + 1] - presModel->strPtr[i]:
                  presModel->storage->text + presModel->storage->len - presModel->strPtr[i];

        TextOut(*hdc, x, y,
                 presModel->strPtr[i],
                 strLength);
    }
}
*/
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
    if(presModel->strPtr != presModel->storage->strPtr) {
        free(presModel->strPtr);
    }

    int newAmount = 0;
    int strLen = 0;
    int i, j, xSize;

    xSize = presModel->cxClient / presModel->cxChar;
    for (i = 1; i < presModel->storage->amount; i++) {
        strLen = (int)(presModel->storage->strPtr[i] - presModel->storage->strPtr[i - 1]);
        newAmount += strLen / xSize + 1;
    }

    char** strings = (char**)calloc(newAmount, sizeof(char*));

    char* bufPtr = 0;
    for (i = 1, j = 0; i < presModel->storage->amount; i++) {
        bufPtr = presModel->storage->strPtr[i - 1];
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

