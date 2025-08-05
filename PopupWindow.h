#ifndef __POPUP_WINDOW_H_
#define __POPUP_WINDOW_H_
#include "BaseWindow.h"

class PopupWindow : public BaseWindow<PopupWindow> {
	// Window Reserved Message
    static const int _nMsg		= 0x400;

    typedef struct tag_MSGMAP {
        UINT iMessage;
        LRESULT(PopupWindow::* lpfnWndProc)(WPARAM, LPARAM);
    }MSGMAP;

    // QUERYENDSESSION
    MSGMAP MainMsg[_nMsg] = {
        {WM_PAINT, &PopupWindow::OnPaint},
		{WM_KEYDOWN, &PopupWindow::OnKeyDown},								// 이 메시지를 처리하거나 창 활성시 포커스를 옮기는 방법으로 변경
		{WM_MOUSEMOVE, &PopupWindow::OnMouseMove},
		{WM_COMMAND, &PopupWindow::OnCommand},
        {WM_SIZE, &PopupWindow::OnSize},
        {WM_MEASUREITEM, &PopupWindow::OnMeasureItem},
        {WM_DRAWITEM, &PopupWindow::OnDrawItem},
        {WM_CREATE, &PopupWindow::OnCreate},
        {WM_DESTROY, &PopupWindow::OnDestroy},
    };

	HWND hComboBox;
	HBITMAP hBitmap;

private:
	void DrawCalendar(HDC hdc, int cx, int cy);
	void DrawMouseTracker(HDC hdc);
    BOOL CheckLeapYear(int Year){
        if(Year % 4 == 0 && Year % 100 != 0){return TRUE;}
        if(Year % 100 == 0 && Year % 400 == 0){return TRUE;}

        return FALSE;
    }

private:
    LPCWSTR ClassName() const { return L"SubWindow Calendar"; }
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(WPARAM wParam, LPARAM lParam);
    LRESULT OnMeasureItem(WPARAM wParam, LPARAM lParam);
    LRESULT OnDrawItem(WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

public:
	PopupWindow();
    ~PopupWindow();

    LRESULT Handler(UINT iMessage, WPARAM wParam, LPARAM lParam);
};

#endif
