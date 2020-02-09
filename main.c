#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <limits.h>
#include <locale.h>
#include "Storage.h"
#include "PresentModel.h"
#include "menu.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    setlocale(LC_ALL, "Rus");

    static char szAppName[] = "Texter";
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.lpszMenuName = MAKEINTRESOURCE(MYMENU);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    HMENU hMenu = LoadMenu(hThisInstance, MYMENU);
    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("Code::Blocks Template Windows App"),
            WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, hMenu, hThisInstance, lpszArgument
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PresentModel presModel;
    static int isOpen;
    HMENU hMenu;
    int i, x, y, iVscrollInc, iHscrollInc;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    TEXTMETRIC tm;
    CREATESTRUCT* pCS;

    wParam = wParam;
    switch (message)                  /* handle the messages */
    {
        case WM_DESTROY:
            FreeModel(&presModel);
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        case WM_CREATE:{
             pCS = (CREATESTRUCT*) lParam;

             char* file =(char*) pCS->lpCreateParams;

             FILE* fp = fopen(file,"rb");
             if (!fp) {
                MessageBox(hwnd, TEXT("Cannot open file"), TEXT("Error"), MB_OK);
                presModel.isOpen = 0;
             }
             else presModel.isOpen = 1;

             hdc = GetDC(hwnd);
             SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));

             if (initPresentModel(&hdc, &presModel, &lParam, fp, IDM_DEFAULT) != 0) {
                MessageBox(hwnd, TEXT("Memory not allocated"), TEXT("Error"), MB_OK);
                PostQuitMessage (0);
                break;
             }

             if (isOpen) fclose(fp);

             ReleaseDC(hwnd, hdc);
             CreateCaret(hwnd, (HBITMAP)0, presModel.cxChar, presModel.cyChar);
             SetCaretPos(presModel.cxChar, presModel.cyChar);
             ShowCaret(hwnd);
             return 0;
        }
        case WM_SIZE : {
            presModel.cxClient = LOWORD(lParam);
            presModel.cyClient = HIWORD(lParam);
            if (presModel.mode == IDM_LAYOUT && reconfigureText(&presModel) != 0) {
                PostQuitMessage (0);
                FreeModel(&presModel);
                break;
            }

            findCaretIndex(&presModel);
            findCaretPosition(&presModel);

            int a = presModel.amount - presModel.cyClient / presModel.cyChar;
            a = max(0, a);
            presModel.iVscrollMax = min(SHRT_MAX, a);

            if (presModel.iVscrollMax != 0) {
                presModel.VCoef = (double)(presModel.amount - presModel.cyClient / presModel.cyChar + 1)
                                   /presModel.iVscrollMax;
            }
            else {
                presModel.VCoef = 1;
            }

            presModel.iVscrollPos = (double)presModel.startLine / presModel.VCoef;
            presModel.iVscrollPos = min(presModel.iVscrollPos, presModel.iVscrollMax);
            SetScrollRange(hwnd, SB_VERT, 0, presModel.iVscrollMax, FALSE);
            SetScrollPos(hwnd, SB_VERT, presModel.iVscrollPos, TRUE);

            if (presModel.mode == IDM_DEFAULT) {
                presModel.iHscrollMax = min(SHRT_MAX,
                                  presModel.storage->maxStrLen
                                  - presModel.cxClient / presModel.cxChar);
                presModel.iHscrollMax = max(presModel.iHscrollMax, 0);
                if (presModel.iHscrollMax != 0) {
                    presModel.HCoef = (double)(presModel.storage->maxStrLen
                                   - presModel.cxClient / presModel.cxChar) / presModel.iHscrollMax;
                }
                else {
                    presModel.HCoef = 1;
                }

                presModel.iHscrollPos = min(presModel.iHscrollPos, presModel.iHscrollMax);
                SetScrollRange(hwnd, SB_HORZ, 0, presModel.iHscrollMax, FALSE);
                SetScrollPos(hwnd, SB_HORZ, presModel.iHscrollPos, TRUE);
            }
            else {
                presModel.iHscrollPos = 0;
                SetScrollRange(hwnd, SB_HORZ, 0, 0, FALSE);
                SetScrollPos(hwnd, SB_HORZ, 0, TRUE);
            }
            return 0;
        }

        case WM_VSCROLL : {
            int prevPos = presModel.iVscrollPos;
            switch(LOWORD(wParam)) {
                case SB_LINEUP :
                    presModel.startLine--;
                    if (presModel.startLine < 0) presModel.startLine = 0;
                    else {
						presModel.iVscrollPos = presModel.startLine / presModel.VCoef;
						ScrollWindow(hwnd, 0, presModel.cyChar, NULL, NULL);
						SetScrollPos(hwnd, SB_VERT, presModel.iVscrollPos, TRUE);
						UpdateWindow(hwnd);
                    }
                    return 0;
                case SB_LINEDOWN :
                    presModel.startLine++;
                    if (presModel.startLine > presModel.amount - presModel.cyClient / presModel.cyChar + 2)
						presModel.startLine = presModel.amount - presModel.cyClient / presModel.cyChar + 2;
                    else {
						presModel.iVscrollPos = presModel.startLine / presModel.VCoef;
						iVscrollInc = presModel.iVscrollPos - prevPos;
						ScrollWindow(hwnd, 0, -presModel.cyChar, NULL, NULL);
						SetScrollPos(hwnd, SB_VERT, presModel.iVscrollPos, TRUE);
						UpdateWindow(hwnd);
                    }
                    return 0;
                case SB_PAGEUP :
                    presModel.iVscrollPos += min(-1, - (int)(presModel.cyClient / (int)presModel.cyChar));
                    break;
                case SB_PAGEDOWN :
                    presModel.iVscrollPos += max(1, presModel.cyClient / presModel.cyChar);
                    break;
                case SB_THUMBTRACK :
                    presModel.iVscrollPos = HIWORD(wParam);
                    break;
				case SB_TOP:
					presModel.iVscrollPos = 0;
					break;
				case SB_BOTTOM:
					presModel.iVscrollPos = presModel.iVscrollMax;
					break;
                default:
                    iVscrollInc = 0;
            }

            if (presModel.iVscrollPos > presModel.iVscrollMax) {
                presModel.iVscrollPos = presModel.iVscrollMax;
            }
            if (presModel.iVscrollPos < 0) {
                presModel.iVscrollPos = 0;
            }

			iVscrollInc = presModel.iVscrollPos - prevPos;
            if (iVscrollInc != 0) {
                int prevStartLine = presModel.startLine;
                presModel.startLine = presModel.VCoef * (double)presModel.iVscrollPos + 1;
                ScrollWindow(hwnd, 0, -(int)(presModel.startLine - prevStartLine)*presModel.cyChar, NULL, NULL);
                SetScrollPos(hwnd, SB_VERT, presModel.iVscrollPos, TRUE);
                UpdateWindow(hwnd);
            }

            return 0;
        }

        case WM_HSCROLL : {
            switch(LOWORD(wParam)) {
                case SB_LINEUP :
                    iHscrollInc = -1;
                    break;
                case SB_LINEDOWN :
                    iHscrollInc = 1;
                    break;
                case SB_PAGEUP :
                    iHscrollInc = min(-1, -(int)(presModel.cxClient / (int)presModel.cxChar));
                    break;
                case SB_PAGEDOWN :
                    iHscrollInc = max(1, presModel.cxClient / presModel.cxChar);
                    break;
                case SB_THUMBTRACK :
                    iHscrollInc = HIWORD(wParam) - presModel.iHscrollPos;
                    break;
                default:
                    iHscrollInc = 0;
            }

            if (iHscrollInc + presModel.iHscrollPos >= presModel.iHscrollMax) {
                iHscrollInc = presModel.iHscrollMax - presModel.iHscrollPos;
            }
            if (iHscrollInc + presModel.iHscrollPos <= 0) {
                iHscrollInc = -presModel.iHscrollPos;
            }
            if (iHscrollInc != 0) {
                presModel.iHscrollPos += iHscrollInc;
                ScrollWindow(hwnd, -(int)(presModel.HCoef*presModel.cxChar*iHscrollInc), 0, NULL, NULL);
                SetScrollPos(hwnd, SB_HORZ, presModel.iHscrollPos, TRUE);
                UpdateWindow(hwnd);
            }
            return 0;
        }

        case WM_PAINT:{
            hdc = BeginPaint(hwnd, &ps);
            PrintText(&hdc, &ps, &presModel);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_KEYDOWN: {
            iVscrollInc = iHscrollInc = 0;

            switch (wParam)
            {
            case VK_UP:
                moveCaretUp(&presModel, hwnd, 0);
                break;
            case VK_DOWN:
                moveCaretDown(&presModel, hwnd, 0);
                break;
            case VK_LEFT:
                moveCaretLeft(&presModel, hwnd);
                break;
            case VK_RIGHT:
                moveCaretRight(&presModel, hwnd);
                break;
            case VK_HOME:
                SendMessage(hwnd, WM_VSCROLL, SB_TOP, 0L);
                break;
            case VK_END:
                SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0L);
                break;
            case VK_PRIOR:
                SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0L);
                break;
            case VK_NEXT:
                SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
                break;
            }

            return 0;
        }
        case WM_COMMAND:
            if (menuAction(&hwnd, wParam, &presModel) != 0) {
                PostQuitMessage (0);
                FreeModel(&presModel);
                break;
            }
            return 0;

        case WM_SETFOCUS:
            CreateCaret(hwnd, (HBITMAP)0, presModel.cxChar, presModel.cyChar);
            SetCaretPos((presModel.caretLetter + presModel.iHscrollPos)*presModel.cxChar,
                 (presModel.caretLine - presModel.startLine)* presModel.cyChar);
            ShowCaret(hwnd);
            break;

        case WM_KILLFOCUS:
            HideCaret(hwnd);
            DestroyCaret();
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }


    return 0;
}
