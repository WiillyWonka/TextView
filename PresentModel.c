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
    presModel->storage = 0;
    presModel->storage = (Storage*)calloc(1, sizeof(Storage));
    if (presModel->storage == 0) {
        return 1;
    }
    presModel->iVscrollPos = presModel->iHscrollPos = 0;
    presModel->VCoef = presModel->HCoef = 1;
    presModel->strPtr = 0;
    presModel->amount = 0;
    presModel->startLine = 0;
    presModel->storage->text = 0;
    presModel->storage->strPtr = 0;

    if (presModel->isOpen) {
		int code = fillStorage(presModel->storage, fp);
		switch (code)
		{
		case 1: return 0;
		case 2: presModel->isOpen = 0;
		}
		presModel->amount = presModel->storage->amount;
		presModel->strPtr = presModel->storage->strPtr;

		if (presModel->mode == IDM_LAYOUT) {
			if (reconfigureText(presModel) != 0) return 1;
		}
    } else {
        presModel->storage->text = (char*) calloc(1, sizeof(char));
        presModel->storage->text[0] = '\0';
        presModel->storage->strPtr = (char**) calloc(1, sizeof(char*));
        presModel->storage->strPtr[0] = presModel->storage->text;
        presModel->storage->amount = 1;
        presModel->storage->len = 1;
        presModel->storage->maxStrLen = 1;
        presModel->amount = presModel->storage->amount;
        presModel->strPtr = presModel->storage->strPtr;
    }

    presModel->caretLetter = 1;
    presModel->caretLine = 1;
    presModel->action = 1;
	presModel->reconfigurate = 1;
	SetCaretPos(presModel->cxChar, presModel->cyChar);
    return 0;
}

void PrintText(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel) {
    int a = presModel->startLine + ps->rcPaint.top / presModel->cyChar - 1;
    int iPaintBeg =  max(0, a);
    a = presModel->startLine + ps->rcPaint.bottom / presModel->cyChar;
    int iPaintEnd = min(presModel->amount, a);
    int i, x, y, strLength;

    if (!presModel->isOpen) iPaintEnd = 0;
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

int reconfigureText(PresentModel* presModel) {
	if (presModel->reconfigurate == 0) {
		presModel->reconfigurate = 1;
		return 0;
	}
    int newAmount = 0;
    int strLen = 0;
    int i, j, xSize;

    findCaretIndex(presModel);

    xSize = presModel->cxClient / presModel->cxChar - 1;
    if (xSize <= 0) {
        return 0;
    }

    for (i = 1; i < presModel->storage->amount; i++) {
        strLen = (int)(presModel->storage->strPtr[i] - presModel->storage->strPtr[i - 1]);
        newAmount += strLen / xSize + 1;
    }
	strLen = (int)(presModel->storage->text + presModel->storage->len - 1 - presModel->storage->strPtr[i - 1]);
	newAmount += strLen / xSize + 1;

    char** strings = (char**)calloc(newAmount, sizeof(char*));
    if (!strings) {
        return 1;
    }

    char* bufPtr = 0;
    char* startPtr = presModel->strPtr[presModel->startLine];
    int minLen = INT_MAX;
    int newLine = presModel->startLine;
    for (i = 1, j = 0; i < presModel->storage->amount; i++) {
        bufPtr = presModel->storage->strPtr[i - 1];
        int delta = bufPtr - startPtr;
        if (delta < minLen && delta <= 0) {
            minLen = abs(startPtr - bufPtr);
            presModel->startLine = j;
        }
        strLen = (int)(presModel->storage->strPtr[i] - presModel->storage->strPtr[i - 1]);
        strings[j] = bufPtr;
        j++;
        while(presModel->storage->strPtr[i] - bufPtr > xSize) {
            bufPtr += xSize;
            strings[j] = bufPtr;
            j++;
        }
    }

	bufPtr = presModel->storage->strPtr[i - 1];
	int delta = bufPtr - startPtr;
    if (delta < minLen && delta <= 0) {
		minLen = abs(startPtr - bufPtr);
		presModel->startLine = j;
	}
	strings[j] = bufPtr;
	j++;
	while (presModel->storage->text + presModel->storage->len - bufPtr > xSize) {
        bufPtr += xSize;
		strings[j] = bufPtr;
		j++;
	}

    if(presModel->strPtr != presModel->storage->strPtr) {
        free(presModel->strPtr);
    }
    presModel->amount = j;
    presModel->strPtr = strings;

    //printf("rec %i\n", presModel->caretIdx);
    findCaretPosition(presModel);

    return 0;
}

void findCaretIndex(PresentModel* presModel) {
    presModel->caretIdx = 0;
    if (presModel->isOpen) {
        for (int k = 0; k < presModel->caretLine - 1; k++) {
            int strLength = (k < presModel->amount - 1)?
                  presModel->strPtr[k + 1] - presModel->strPtr[k]:
                  presModel->storage->text + presModel->storage->len - presModel->strPtr[k];
            presModel->caretIdx += strLength;
        }
        presModel->caretIdx += presModel->caretLetter - 1;
        //printf("caretIdx = %i\n", caretIdx);
    }
}

void findCaretPosition(PresentModel* presModel) {
    if (presModel->isOpen && presModel->action != 0) {
        for (int i = 1; i < presModel->amount; i++) {
            int lineStartIdx = presModel->strPtr[i - 1] - presModel->storage->text;
            int lineEndIdx = presModel->strPtr[i] - presModel->storage->text;
            if (presModel->caretIdx >= lineStartIdx && presModel->caretIdx <= lineEndIdx) {
                presModel->caretLine = i;
                presModel->caretLetter = presModel->caretIdx - lineStartIdx + 1;
                /*printf("lineStartIdx = %i\n", lineStartIdx);
                printf("lineEndIdx = %i\n", lineEndIdx);
                printf("Line = %i\n", presModel->caretLine);
                printf("Letter = %i\n", presModel->caretLetter);*/
            }
        }


        int lineStartIdx = presModel->strPtr[presModel->amount - 1] - presModel->storage->text;
        int lineEndIdx = presModel->storage->len - 1;
        if (presModel->caretIdx >= lineStartIdx && presModel->caretIdx <= lineEndIdx) {
            presModel->caretLine = presModel->amount;
            presModel->caretLetter = presModel->caretIdx - lineStartIdx + 1;
            /*printf("lineStartIdx = %i\n", lineStartIdx);
            printf("lineEndIdx = %i\n", lineEndIdx);
            printf("Line = %i\n", presModel->caretLine);
            printf("Letter = %i\n", presModel->caretLetter);*/
        }

        SetCaretPos((presModel->caretLetter - presModel->iHscrollPos)*presModel->cxChar,
                     (presModel->caretLine - presModel->startLine)*presModel->cyChar);
    }
    presModel->action = 1;
}

void FreeModel(PresentModel* presModel) {
    if (presModel) {
        switch (presModel->mode) {
        case IDM_DEFAULT: {
            if (presModel->storage->text) free(presModel->storage->text);
            if (presModel->strPtr) free(presModel->strPtr);
            free(presModel->storage);
            break;
        }
        case IDM_LAYOUT:{
            if (presModel->strPtr) free(presModel->strPtr);
            if (presModel->storage->text) free(presModel->storage->text);
            if (presModel->storage->strPtr) free(presModel->storage->strPtr);
            free(presModel->storage);
            break;
        }
        }
    }
}

void moveCaretUp(PresentModel* presModel, HWND hwnd, int lineBreak) {
    if (!presModel->isOpen) return;
    presModel->caretLine--;

    if (presModel->caretLine < 1) {
        presModel->caretLine = 1;
        moveToCaret(presModel, hwnd);
        SetCaretPos((presModel->caretLetter - presModel->iHscrollPos)*presModel->cxChar,
                 (presModel->caretLine - presModel->startLine)*presModel->cyChar);
        return;
    }
    else if (presModel->caretLine <= presModel->startLine) {
        SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
    }

    int curLineSize = presModel->strPtr[presModel->caretLine] - presModel->strPtr[presModel->caretLine - 1];
    if (presModel->caretLetter > curLineSize || presModel->caretLetter < 1 || lineBreak) {
        presModel->caretLetter = curLineSize;
    }

    moveToCaret(presModel, hwnd);
    SetCaretPos((presModel->caretLetter - presModel->iHscrollPos)*presModel->cxChar,
                 (presModel->caretLine - presModel->startLine)*presModel->cyChar);
}

void moveCaretDown(PresentModel* presModel, HWND hwnd, int lineBreak) {
    int eof = 0;
    if (!presModel->isOpen) return;
    presModel->caretLine++;

    if (presModel->caretLine > presModel->amount) {
        presModel->caretLine = presModel->amount;
        eof = 1;
    }
    else if (presModel->caretLine > presModel->startLine + presModel->cyClient / presModel->cyChar - 1) {
        SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
    }

	int curLineSize = presModel->caretLine == presModel->amount ?
		presModel->storage->text + presModel->storage->len - presModel->strPtr[presModel->caretLine - 1] :
		presModel->strPtr[presModel->caretLine] - presModel->strPtr[presModel->caretLine - 1];
	if (presModel->caretLetter > curLineSize) presModel->caretLetter = curLineSize;
	if ((presModel->caretLetter < 1 || lineBreak) && !eof) presModel->caretLetter = 1;

    moveToCaret(presModel, hwnd);
    SetCaretPos((presModel->caretLetter - presModel->iHscrollPos)*presModel->cxChar,
                 (presModel->caretLine - presModel->startLine)* presModel->cyChar);
}

void moveCaretLeft(PresentModel* presModel, HWND hwnd) {
    if (!presModel->isOpen) return;
    int curLineSize = 0;
	curLineSize = (presModel->caretLine == presModel->amount ?
		(presModel->storage->text + presModel->storage->len - 1) - presModel->strPtr[presModel->caretLine - 1] :
		presModel->strPtr[presModel->caretLine] - presModel->strPtr[presModel->caretLine - 1]);

    if (presModel->mode == IDM_LAYOUT && presModel->caretLetter > presModel->cxClient / presModel->cxChar)
        presModel->caretLetter = presModel->cxClient / presModel->cxChar + 1;
    if (presModel->caretLetter <= 1) moveCaretUp(presModel, hwnd, 1);
    else  {
        presModel->caretLetter--;
		if (presModel->caretLetter <= presModel->iHscrollPos * presModel->HCoef) {
			SendMessage(hwnd, WM_HSCROLL, SB_LINEUP, 0L);
		}
        moveToCaret(presModel, hwnd);
        SetCaretPos((presModel->caretLetter - presModel->iHscrollPos)*presModel->cxChar,
                 (presModel->caretLine - presModel->startLine)* presModel->cyChar);
    }
    /*printf("curLineSize = %i\n", curLineSize);
    printf("presModel->caretLetter = %i\n", presModel->caretLetter);*/

}

void moveCaretRight(PresentModel* presModel, HWND hwnd) {
    if (!presModel->isOpen) return;
	int curLineSize = 0;
	curLineSize = (presModel->caretLine == presModel->amount ?
		(presModel->storage->text + presModel->storage->len) - presModel->strPtr[presModel->caretLine - 1] :
		presModel->strPtr[presModel->caretLine] - presModel->strPtr[presModel->caretLine - 1]);

    if (presModel->mode == IDM_LAYOUT) curLineSize = min(presModel->cxClient / presModel->cxChar, curLineSize);
    if (presModel->caretLetter > curLineSize - 1) {
        moveCaretDown(presModel, hwnd, 1);
    }
    else  {
        presModel->caretLetter++;
		if (presModel->caretLetter >
			presModel->iHscrollPos * presModel->HCoef + presModel->cxClient / presModel->cxChar - 1 &&
            presModel->mode == IDM_DEFAULT)
		{
			SendMessage(hwnd, WM_HSCROLL, SB_LINEDOWN, 0L);
		}
		moveToCaret(presModel, hwnd);
		SetCaretPos((presModel->caretLetter - presModel->iHscrollPos) * presModel->cxChar,
			(presModel->caretLine - presModel->startLine) * presModel->cyChar);
    }
}

void moveToCaret(PresentModel* presModel, HWND hwnd) {
    int firstSimb = presModel->iHscrollPos * presModel->HCoef;
    //printf("%i\n", presModel->caretLine);
    //printf("%i\n", presModel->startLine);
    if (presModel->caretLine <= presModel->startLine ||
        presModel->caretLine >= presModel->startLine + presModel->cyClient / presModel->cyChar ||
        presModel->caretLetter < firstSimb ||
        (presModel->caretLetter >= firstSimb + presModel->cxClient / presModel->cxChar + 1) &&
        presModel->mode == IDM_DEFAULT)
    {
        /*if (presModel->mode == IDM_DEFAULT) {
            presModel->iVscrollPos = (presModel->caretLine - 1) / presModel->VCoef;
            presModel->iVscrollPos = min(presModel->iVscrollPos, presModel->iVscrollMax);
            presModel->startLine = presModel->iVscrollPos * presModel->VCoef + 1;
        } else if (presModel->mode == IDM_LAYOUT) {

            findCaretIndex(presModel);
            while
        }*/

        presModel->iVscrollPos = (double)(presModel->caretLine - 1) / presModel->VCoef + 1;
        presModel->iVscrollPos = min(presModel->iVscrollPos, presModel->iVscrollMax);
        presModel->startLine = (double)(presModel->caretLine - 1);
		presModel->startLine = max(presModel->startLine, 0);


        presModel->iHscrollPos = (presModel->caretLetter - 1 - presModel->cxClient / (presModel->cyChar * 2))
                                / presModel->HCoef;
        presModel->iHscrollPos = max(0, presModel->iHscrollPos);
        presModel->iHscrollPos = min(presModel->iHscrollPos, presModel->iHscrollMax);

        SetScrollPos(hwnd, SB_VERT, presModel->iVscrollPos, TRUE);
        SetScrollPos(hwnd, SB_HORZ, presModel->iHscrollPos, TRUE);
        presModel->action = 0; //Костыль для корректной работы движения ползунка за буквой
		presModel->reconfigurate = 0;
		int newLPARAM = (presModel->cyClient << 16) + presModel->cxClient;
		InvalidateRect(hwnd, NULL, TRUE);
		//findCaretIndex(presModel);
		//printf("A1 %i\n", presModel->caretLine);
        //printf("B1 %i\n", presModel->startLine);
		SendMessage(hwnd, WM_SIZE, 0, newLPARAM);
		//printf("A2 %i\n", presModel->caretLine);
		presModel->reconfigurate = 1;
    }
}
