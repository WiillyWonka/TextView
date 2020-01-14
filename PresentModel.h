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
    iVscrollPos, iHscrollPos, startLine;
    double VCoef, HCoef;
    char** strPtr;
    int mode, isOpen;
    int caretLine, caretLetter;
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
void moveCaretUp(PresentModel*, HWND);
void moveCaretDown(PresentModel*, HWND);
void moveCaretLeft(PresentModel*, HWND);
void moveCaretRight(PresentModel*, HWND);

/*����������� ������� ������� � ��������� �������
in: HandleWindow
inout: Presenr Model
*/
void moveToCaret(PresentModel*, HWND);
