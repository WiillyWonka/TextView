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

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("Code::Blocks Template Windows App"),
            WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, hThisInstance, lpszArgument               /* No Window Creation data */
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
    static int iHscrollMax, iVscrollMax;
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
                break;
             }


             hdc = GetDC(hwnd);
             SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));

             initPresentModel(&hdc, &presModel, &lParam);
             fillPresentModel(&presModel, fp);

             /*if (presModel.layoutMode) {
                reconfigureText(&presModel);
            }*/

             fclose(fp);

             ReleaseDC(hwnd, hdc);
             return 0;
        }
        case WM_SIZE : {
            presModel.cxClient = LOWORD(lParam);
            presModel.cyClient = HIWORD(lParam);

            if (presModel.layoutMode) {
                reconfigureText(&presModel);
            }


            iVscrollMax = min(SHRT_MAX, presModel.amount - presModel.cyClient / presModel.cyChar);

            if (iVscrollMax != 0) {
                presModel.VCoef = (double)(presModel.amount - presModel.cyClient / presModel.cyChar + 1)
                                   /iVscrollMax;
            }
            else {
                presModel.VCoef = 1;
            }

            presModel.iVscrollPos = min(presModel.iVscrollPos, iVscrollMax);
            SetScrollRange(hwnd, SB_VERT, 0, iVscrollMax, FALSE);
            SetScrollPos(hwnd, SB_VERT, presModel.iVscrollPos, TRUE);

            if (!presModel.layoutMode) {
                iHscrollMax = min(SHRT_MAX,
                                  presModel.storage->maxStrLen
                                  - presModel.cxClient / presModel.cxChar);
                iHscrollMax = max(iHscrollMax, 0);
                if (iHscrollMax != 0) {
                    presModel.HCoef = (double)(presModel.storage->maxStrLen
                                   - presModel.cxClient / presModel.cxChar) / iHscrollMax;
                }
                else {
                    presModel.HCoef = 1;
                }

                presModel.iHscrollPos = min(presModel.iHscrollPos, iHscrollMax);
                SetScrollRange(hwnd, SB_HORZ, 0, iHscrollMax, FALSE);
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
            switch(LOWORD(wParam)) {
                case SB_LINEUP :
                    iVscrollInc = -1;
                    break;
                case SB_LINEDOWN :
                    iVscrollInc = 1;
                    break;
                case SB_PAGEUP :
                    iVscrollInc = min(-1, - (int)(presModel.cyClient / (int)presModel.cyChar));
                    break;
                case SB_PAGEDOWN :
                    iVscrollInc = max(1, presModel.cyClient / presModel.cyChar);
                    break;
                case SB_THUMBTRACK :
                    iVscrollInc = HIWORD(wParam)- presModel.iVscrollPos;
                    break;
                default:
                    iVscrollInc = 0;
            }

            if (iVscrollInc + presModel.iVscrollPos >= iVscrollMax) {
                iVscrollInc = iVscrollMax - presModel.iVscrollPos;
            }
            if (iVscrollInc + presModel.iVscrollPos <= 0) {
                iVscrollInc = -(int)(presModel.iVscrollPos);
            }

            if (iVscrollInc != 0) {
                presModel.iVscrollPos += iVscrollInc;
                ScrollWindow(hwnd, 0, -presModel.VCoef*presModel.cyChar*iVscrollInc, NULL, NULL);
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

            if (iHscrollInc + presModel.iHscrollPos >= iHscrollMax) {
                iHscrollInc = iHscrollMax - presModel.iHscrollPos;
            }
            if (iHscrollInc + presModel.iHscrollPos <= 0) {
                iHscrollInc = -presModel.iHscrollPos;
            }
            //printf("%d", iHscrollInc);
            if (iHscrollInc != 0) {
                presModel.iHscrollPos += iHscrollInc;
                ScrollWindow(hwnd, -(presModel.HCoef*presModel.cxChar*iHscrollInc), 0, NULL, NULL);
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
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
