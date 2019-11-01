#include <stdio.h>
#include "PresentModel.h"
#include "menu.h"

void menuAction(HWND hwnd, WPARAM wParam, PresentModel* presModel) {
    HMENU hMenu = GetMenu(hwnd);
    switch (LOWORD(wParam)) {
        case IDM_DEFAULT: {
            printf("%i\n", presModel->layoutMode);
            CheckMenuItem(hMenu, presModel->layoutMode, MF_UNCHECKED);
            presModel->layoutMode = LOWORD(wParam);
            CheckMenuItem(hMenu, presModel->layoutMode, MF_CHECKED);

            AdjustView(Text, Text->maxLineLen * Text->letterWidth);
            Text->alignment = 0;

            SendMessage(hwnd, WM_SIZE, 0, 0L);

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case IDM_LAYOUT: {
            CheckMenuItem(hMenu, presModel->layoutMode, MF_UNCHECKED);
            presModel->layoutMode = LOWORD(wParam);
            CheckMenuItem(hMenu, presModel->layoutMode, MF_CHECKED);
            break;
        }
        case IDM_EXIT: {
            FreeModel(presModel);
            PostQuitMessage(0);
            break;
        }
    }
}
