#pragma once

#include <windows.h>
#include "Storage.h"

/*������ ������������� ������
������:
    ������ �������� ������
    ������� ������� �������
    ������� ��������
    ��������� �� ������ �����
    ���� ����v� �����������
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
//������ ������
//in: HDC*, PAINSTRUCT*, PresentModel*
void PrintText(HDC* hdc, PAINTSTRUCT* ps, PresentModel* presModel);

//������������ ���������� ������
//in/out: PresentModel*
void FreeModel(PresentModel* presModel);
//������������� ������ ��� ������ ������
//inout: PresentModel*
//out: success code
int reconfigureText(PresentModel* presModel);

/*�������� �������
in: HandleWindow
inout: Presenr Model
*/
void moveCaretUp(PresentModel*, HWND, int lineBreak);
void moveCaretDown(PresentModel*, HWND, int lineBreak);
void moveCaretLeft(PresentModel*, HWND);
void moveCaretRight(PresentModel*, HWND);

/*����������� ������� ������� � ��������� �������
in: HandleWindow
inout: Presenr Model
*/
void moveToCaret(PresentModel*, HWND);

/*����� ������� �������, ����� �������� ����� �������
inout: Present Model
*/
void findCaretIndex(PresentModel*);
/*����� ������� �������
inout: Present Model
*/
void findCaretPosition(PresentModel*);
