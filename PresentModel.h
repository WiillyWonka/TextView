#pragma once

#include <windows.h>
#include "Storage.h"

/*Модель представления данных
хранит:
    модель хранения данных
    метрики рабочей области
    метрики ползунка
    указатели на начала строк
    флаг режиvа отображения
*/
typedef struct PresentModel {
    Storage* storage;
    int cxClient, cyClient, cxChar, cyChar, amount,
    iVscrollPos, iHscrollPos, iVscrollMax, iHscrollMax, startLine;
    double VCoef, HCoef;
    char** strPtr;
    int mode, isOpen;
    int caretLine, caretLetter, caretIdx;
    int action, reconfigurate;
} PresentModel;

//Initiate PresentModel
//in: HDC* - handleDC, LPARAM , FILE*, int mode
//inout: PresentModel
//out: success code
int initPresentModel(HDC* hdc, PresentModel* presModel, LPARAM* lParam,
                      FILE* fp, int mode);
//Печать текста
//in: HDC*, PAINSTRUCT*, PresentModel*
void PrintText(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel);

//Освобождение выделенной памяти
//in/out: PresentModel*
void FreeModel(PresentModel* presModel);
//Перестраивает строки для режима вёрстки
//inout: PresentModel*
//out: success code
int reconfigureText(PresentModel* presModel);

/*Движение каретки
in: HandleWindow
inout: Presenr Model
*/
void moveCaretUp(PresentModel*, HWND, int lineBreak);
void moveCaretDown(PresentModel*, HWND, int lineBreak);
void moveCaretLeft(PresentModel*, HWND);
void moveCaretRight(PresentModel*, HWND);

/*Перемещение рабочей области к положению каретки
in: HandleWindow
inout: Presenr Model
*/
void moveToCaret(PresentModel*, HWND);

/*Поиск индекса символа, возле которого стоит каретка
inout: Present Model
*/
void findCaretIndex(PresentModel*);
/*Поиск позиции каретки
inout: Present Model
*/
void findCaretPosition(PresentModel*);
