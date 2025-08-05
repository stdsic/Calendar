#include "PopupWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    PopupWindow popwin;
    if(popwin.Create(L"Calendar", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL)){
        popwin.RunMessageLoop();
    }
    return 0;
}
