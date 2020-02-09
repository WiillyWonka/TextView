#include <stdio.h>
#include <limits.h>
#include <tchar.h>
#include <windows.h>
#include "PresentModel.h"
#include "menu.h"

void InitOFN(OPENFILENAME * ofn, HWND * hwnd)
{
    ZeroMemory(ofn, sizeof(*ofn));
    static CHAR szFilter[] = "Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0";
    ofn->lStructSize = sizeof(*ofn);
    ofn->hwndOwner = *hwnd;
    ofn->lpstrFile = "\0";
    ofn->nMaxFile = 100;
    ofn->lpstrFilter = (LPCSTR)szFilter;
    ofn->nFilterIndex = 1;
    ofn->lpstrTitle = TEXT("Open");
    ofn->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
 }

int menuAction(HWND* hwnd, WPARAM wParam, PresentModel* presModel) {
    HMENU hMenu = GetMenu(*hwnd);
    int newLPARAM  = (presModel->cyClient << 16) + presModel->cxClient;
    switch (LOWORD(wParam)) {
        case IDM_OPEN: {
            OPENFILENAME ofn;
            char nameFile[1000] = { 0 };
            InitOFN(&ofn, hwnd);
            ofn.lpstrFile = nameFile;
            if (GetOpenFileName((LPOPENFILENAME)&ofn)) {
                FreeModel(presModel);
                FILE *fp;
                fp = fopen(ofn.lpstrFile, "rb");
                if (fp == 0) {
                    MessageBox(hwnd, TEXT("Cannot open file"), TEXT("Error"), MB_OK);
                    return 1;
                }
                presModel->isOpen = 1;
                if (initPresentModel(0, presModel, 0, fp, presModel->mode) != 0) return 1;
                fclose(fp);
                SendMessage(*hwnd, WM_SIZE, 0, newLPARAM);
                InvalidateRect(*hwnd, NULL, TRUE);
            }
            break;
        }
        case IDM_DEFAULT: {
            if (presModel->mode == IDM_DEFAULT) break;
            CheckMenuItem(hMenu, presModel->mode, MF_UNCHECKED);
            presModel->mode = LOWORD(wParam);
            CheckMenuItem(hMenu, presModel->mode, MF_CHECKED);

            findCaretIndex(presModel);

            char* startPtr = presModel->strPtr[presModel->startLine];

            free(presModel->strPtr);
            presModel->strPtr = presModel->storage->strPtr;
            presModel->amount = presModel->storage->amount;

            char* bufPtr = 0;
            int minLen = INT_MAX;
            int newLine = presModel->startLine;
            for (int i = 0; i < presModel->amount; i++) {
                bufPtr = presModel->strPtr[i];
                if (abs(startPtr - bufPtr) < minLen) {
                    minLen = abs(startPtr - bufPtr);
                    presModel->startLine = i;
                }
            }
            findCaretPosition(presModel);
            SendMessage(*hwnd, WM_SIZE, 0, newLPARAM);

            InvalidateRect(*hwnd, NULL, TRUE);
            break;
        }
        case IDM_LAYOUT: {
            if (presModel->mode == IDM_LAYOUT) break;
            CheckMenuItem(hMenu, presModel->mode, MF_UNCHECKED);
            presModel->mode = LOWORD(wParam);
            CheckMenuItem(hMenu, presModel->mode, MF_CHECKED);
            SendMessage(*hwnd, WM_SIZE, 0, newLPARAM);
            InvalidateRect(*hwnd, NULL, TRUE);
            break;
        }
        case IDM_EXIT: {
            FreeModel(presModel);
            PostQuitMessage(0);
            break;
        }
    }
    return 0;
}
