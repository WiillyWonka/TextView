#include "PresentModel.h"

#define MYMENU 42

#define IDM_OPEN    1
#define IDM_EXIT    2
#define IDM_DEFAULT 3
#define IDM_LAYOUT 4

HWND hwndDialog;

void menuAction(HWND hwnd, WPARAM wParam, PresentModel* presModel);
