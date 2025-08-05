#include "PopupWindow.h"
#include <strsafe.h>
#define IDC_COMBOBOX    0x101

// PopupWindow Init
PopupWindow::PopupWindow(){
	// 생성 및 초기화
}

PopupWindow::~PopupWindow(){
	// 삭제 및 해제
}

// PopupWindow Message
LRESULT PopupWindow::Handler(UINT iMessage, WPARAM wParam, LPARAM lParam){
	DWORD i;

    for(i=0; i<sizeof(MainMsg) / sizeof(MainMsg[0]); i++) {
        if (MainMsg[i].iMessage == iMessage) {
            return (this->*MainMsg[i].lpfnWndProc)(wParam, lParam);
        }
    }

    return DefWindowProc(_hWnd, iMessage, wParam, lParam);
}

LRESULT PopupWindow::OnMeasureItem(WPARAM wParam, LPARAM lParam){
	LPMEASUREITEMSTRUCT lpmis	= (LPMEASUREITEMSTRUCT)lParam;
	lpmis->itemHeight			= GetSystemMetrics(SM_CYVSCROLL);
	return TRUE;
}

LRESULT PopupWindow::OnDrawItem(WPARAM wParam, LPARAM lParam){
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
	WCHAR DatePickerItem[256];

	int x,y;
	SIZE TextSize;

	switch(lpdis->CtlID){
		case IDC_COMBOBOX:
			// 콤보 박스에 아이템이 없으면 -1이며, 보통 컨트롤이 포커스를 가졌을 때이다.
			if(lpdis->itemID == -1){
				SYSTEMTIME st;
				GetLocalTime(&st);
				int Year	= st.wYear,
					Month	= st.wMonth;

				StringCbPrintf(DatePickerItem, sizeof(DatePickerItem), L"%d년 - %d월", Year, Month);
				GetTextExtentPoint32(lpdis->hDC, DatePickerItem, wcslen(DatePickerItem), &TextSize);
				x = (lpdis->rcItem.right - lpdis->rcItem.left - TextSize.cx) / 2;
				y = (lpdis->rcItem.bottom - lpdis->rcItem.top - TextSize.cy) / 2;
				TextOut(lpdis->hDC, lpdis->rcItem.left + x, lpdis->rcItem.top + y, DatePickerItem, wcslen(DatePickerItem));
			}else if(lpdis->itemID >= 0){
				// 콤보 박스에 아이템이 있으면 인덱스 값을 가진다
				SetBkColor(lpdis->hDC, GetSysColor(COLOR_WINDOW));
				SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
				FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)(COLOR_WINDOW+1));

				if(lpdis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)){
					SendMessage(hComboBox, CB_GETLBTEXT, (WPARAM)lpdis->itemID, (LPARAM)DatePickerItem);
					GetTextExtentPoint32(lpdis->hDC, DatePickerItem, wcslen(DatePickerItem), &TextSize);
					x = (lpdis->rcItem.right - lpdis->rcItem.left - TextSize.cx) / 2;
					y = (lpdis->rcItem.bottom - lpdis->rcItem.top - TextSize.cy) / 2;
					TextOut(lpdis->hDC, lpdis->rcItem.left + x, lpdis->rcItem.top + y, DatePickerItem, wcslen(DatePickerItem));
				}else if(lpdis->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS)){
					HPEN	hOldPen,
							hPen		= CreatePen(PS_DOT, 1, RGB(0,255,0));
					HBRUSH	hOldBrush	= (HBRUSH)SelectObject(lpdis->hDC, GetStockObject(NULL_BRUSH));
							hOldPen		= (HPEN)SelectObject(lpdis->hDC, hPen);

					Rectangle(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom);

					SendMessage(hComboBox, CB_GETLBTEXT, (WPARAM)lpdis->itemID, (LPARAM)DatePickerItem);
					GetTextExtentPoint32(lpdis->hDC, DatePickerItem, wcslen(DatePickerItem), &TextSize);
					x = (lpdis->rcItem.right - lpdis->rcItem.left - TextSize.cx) / 2;
					y = (lpdis->rcItem.bottom - lpdis->rcItem.top - TextSize.cy) / 2;
					TextOut(lpdis->hDC, lpdis->rcItem.left + x, lpdis->rcItem.top + y, DatePickerItem, wcslen(DatePickerItem));

					SelectObject(lpdis->hDC, hOldPen);
					SelectObject(lpdis->hDC, hOldBrush);

					DeleteObject(hPen);
				}
			}
			break;

	}

	return TRUE;
}

LRESULT PopupWindow::OnSize(WPARAM wParam, LPARAM lParam){
	RECT crt, srt;
	int ComboBoxButtonWidth		= GetSystemMetrics(SM_CXVSCROLL),
		ComboBoxButtonHeight	= GetSystemMetrics(SM_CYVSCROLL);

	if(SIZE_MINIMIZED != wParam){
		GetClientRect(_hWnd, &crt);
		SetRect(&srt, ComboBoxButtonWidth, ComboBoxButtonHeight, (crt.right - crt.left) - ComboBoxButtonWidth * 2, (crt.bottom - crt.top) - ComboBoxButtonHeight);
		SetWindowPos(hComboBox, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);

		if(hBitmap){
			DeleteObject(hBitmap);
			hBitmap = NULL;
		}

	}
	return 0;
}

LRESULT PopupWindow::OnKeyDown(WPARAM wParam, LPARAM lParam){
	WORD VKCode,
		  KeyFlags,
		  ScanCode,
		  RepeatCount;

	BOOL bExtended,
		 bWasKeyDown;

	// 추후 확장시 사용(모드 변경)
	VKCode		= LOWORD(wParam);
	KeyFlags	= HIWORD(lParam);
	ScanCode	= LOBYTE(KeyFlags);
	bExtended	= ((KeyFlags&& KF_EXTENDED) == KF_EXTENDED);

	// 확장 키 플래그 있을 시 0xE0이 접두(HIWORD)로 붙는다
	if(bExtended){ ScanCode = MAKEWORD(ScanCode, 0xE0); }

	bWasKeyDown = ((KeyFlags & KF_REPEAT) == KF_REPEAT);
	RepeatCount = LOWORD(lParam);

	// 특정 키입력은 리스트 박스로 전달
	switch(VKCode){
		case VK_UP:
		case VK_LEFT:
		case VK_DOWN:
		case VK_RIGHT:
			SendMessage(hComboBox, WM_KEYDOWN, wParam, lParam);
			break;

		case VK_ESCAPE:
			DestroyWindow(_hWnd);
			break;
	}

	return 0;
}

LRESULT PopupWindow::OnCommand(WPARAM wParam, LPARAM lParam){
	int Index,
		Length;

	switch(LOWORD(wParam)){
		case IDC_COMBOBOX:
			switch(HIWORD(wParam)){
				case CBN_SELCHANGE:
					InvalidateRect(_hWnd, NULL, FALSE);
					break;
			}
			break;
	}

	return 0;
}

LRESULT PopupWindow::OnPaint(WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(_hWnd, &ps);
	
	// PopupWindow의 작업영역 내에서 콤보박스 위치
	RECT PopupWindowRect, ComboBoxRect;
	GetWindowRect(hComboBox, &ComboBoxRect);
	ScreenToClient(_hWnd, (LPPOINT)&ComboBoxRect);
	ScreenToClient(_hWnd, (LPPOINT)&ComboBoxRect + 1);

	// PopupWindow의 작업영역 크기
	GetClientRect(_hWnd, &PopupWindowRect);

	// 비트맵 생성
	static RECT BitmapRect;

	HDC hMemDC		= CreateCompatibleDC(hdc);
	if(hBitmap == NULL){
		int BitmapWidth		= PopupWindowRect.right;
		int BitmapHeight	= PopupWindowRect.bottom - ComboBoxRect.bottom;
		hBitmap				= CreateCompatibleBitmap(hdc, BitmapWidth, BitmapHeight);

		// hMemDC가 내부 설정 고려
		SetRect(&BitmapRect, 0,0, BitmapWidth, BitmapHeight);
	}
	HGDIOBJ hOld	= SelectObject(hMemDC, hBitmap);
	// COMMENT:	hMemDC는 선택된 리소스에 따라 적절한 정보를 유지/관리하는데
	//			지금처럼 DC에 비트맵을 선택하면(=교체, 끼워넣음) 내부적으로 DC가 관리하는 화면 영역을 비트맵 좌상단에 맞추는 것으로 보인다.
	FillRect(hMemDC, &BitmapRect, GetSysColorBrush(COLOR_BTNFACE));

	// 달력
	DrawCalendar(hMemDC, BitmapRect.right - BitmapRect.left, BitmapRect.bottom - BitmapRect.top);
	// 마우스 트래커
	DrawMouseTracker(hMemDC);
	
	// 출력 위치 설정
	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	BitBlt(hdc, 0, ComboBoxRect.bottom, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	// 리소스 정리
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	EndPaint(_hWnd, &ps);
	return 0;
}

LRESULT PopupWindow::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT crt, wrt, ort, srt;

    HWND hParent = GetParent(_hWnd);
    if(!hParent){
        SystemParametersInfo(SPI_GETWORKAREA, 0, &ort, 0); // 화면 작업 영역 사용
    }else{
        GetWindowRect(hParent, &ort);
    }
	int OwnerWidth	= ort.right - ort.left;
	int OwnerHeight	= ort.bottom - ort.top;
	
	DWORD dwStyle	= GetWindowLongPtr(_hWnd, GWL_STYLE);
	DWORD dwExStyle	= GetWindowLongPtr(_hWnd, GWL_EXSTYLE);

	SetRect(&srt, 0,0, OwnerWidth / 2, OwnerHeight / 2);
	AdjustWindowRectEx(&srt, dwStyle, GetMenu(_hWnd) != NULL, dwExStyle);
	if(dwStyle & WS_VSCROLL){ srt.right += GetSystemMetrics(SM_CXVSCROLL); }
	if(dwStyle & WS_HSCROLL){ srt.bottom += GetSystemMetrics(SM_CYHSCROLL); }

	int CalendarWidth	= srt.right - srt.left;
	int CalendarHeight	= srt.bottom - srt.top;
	SetRect(&crt, ort.left + (OwnerWidth - CalendarWidth) / 2, ort.top + (OwnerHeight - CalendarHeight) / 2, CalendarWidth, CalendarHeight);
	SetWindowPos(_hWnd, NULL, crt.left, crt.top, crt.right, crt.bottom, SWP_NOZORDER);

	int ComboBoxButtonWidth		= GetSystemMetrics(SM_CXVSCROLL);
	int ComboBoxButtonHeight	= GetSystemMetrics(SM_CYVSCROLL);

	GetClientRect(_hWnd, &crt);
	SetRect(&srt, crt.left + ComboBoxButtonWidth, crt.top + ComboBoxButtonHeight, (crt.right - crt.left) - ComboBoxButtonWidth * 2, (crt.bottom - crt.top) - ComboBoxButtonHeight);
	hComboBox = CreateWindow(L"combobox", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 0,0,0,0, _hWnd, (HMENU)IDC_COMBOBOX, GetModuleHandle(NULL), NULL);
	SetWindowPos(hComboBox, NULL, srt.left, srt.top, srt.right, srt.bottom, SWP_NOZORDER);

	SYSTEMTIME st;
	GetLocalTime(&st);
	int Year	= st.wYear,
		Month	= st.wMonth,
		Day		= st.wDay;

	int YearRange	= 5,
		MonthRange	= 12;

	TCHAR DatePickerItem[256];
	for(int i=0; i<YearRange * 2; i++){
		for(int j=1; j<=MonthRange; j++){
			StringCbPrintf(DatePickerItem, sizeof(DatePickerItem), L"%d년 - %d월", (Year - YearRange)+ i, j);
			SendDlgItemMessage(_hWnd, IDC_COMBOBOX, CB_ADDSTRING, 0, (LPARAM)DatePickerItem);
		}
	}
	return 0;
}

LRESULT PopupWindow::OnDestroy(WPARAM wParam, LPARAM lParam){
	if(hBitmap){DeleteObject(hBitmap);}
    if(GetWindowLongPtr(_hWnd, GWL_STYLE) & WS_OVERLAPPEDWINDOW){
        PostQuitMessage(0);
    }
	return 0;
}

LRESULT PopupWindow::OnMouseMove(WPARAM wParam, LPARAM lParam){
	InvalidateRect(_hWnd, NULL, FALSE);
	return 0;
}

void PopupWindow::DrawCalendar(HDC hdc, int cx, int cy){
	static int Days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	static CONST WCHAR* DayOfTheWeek[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };

	int x = 0,
		y = 0,
		Index,
		Length,
		Year,
		Month,
		Day,
		LastDay,
		DayOfWeek,
		Div,
		DivGap,
		RowGap,
		ColumnGap;

	BOOL bTrue;
	RECT CellRect;
	WCHAR buf[64];
	SIZE TextSize;

	FILETIME ft;
	SYSTEMTIME st, today;

	GetLocalTime(&today);

	Index = SendMessage(hComboBox, CB_GETCURSEL, 0,0);
	if(Index == CB_ERR){
		Year = today.wYear;
		Month = today.wMonth;
		StringCbPrintf(buf, sizeof(buf), L"%d년 - %d월", Year, Month);
		Index = SendMessage(hComboBox, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
		SendMessage(hComboBox, CB_SETCURSEL, (WPARAM)Index, (LPARAM)0);
	}else{
		SendMessage(hComboBox, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)buf);

		for(int i=0; buf[i]; i++){
			if(buf[i] >= '0' && buf[i] <= '9'){
				x*= 10;
				x+= buf[i] - '0';
			}else if(buf[i] == '-'){
				Year	= x;
				x		= 0;
			}
		}

		Month = x;
	}

	if(Month == 2 && CheckLeapYear(Year)){
		LastDay = 29;
	}else{
		LastDay = Days[Month];
	}

	memset(&st, 0, sizeof(st));
	st.wYear	= Year;
	st.wMonth	= Month;
	st.wDay		= 1;
	SystemTimeToFileTime(&st, &ft);
	FileTimeToSystemTime(&ft, &st);
	
	DayOfWeek	= st.wDayOfWeek;

	HPEN hPen = CreatePen(PS_DOT, 1, RGB(255,255,255)),
		 hOldPen = (HPEN)SelectObject(hdc, hPen);

	Div		= 7;
	DivGap	= cx / Div;
	for(int i=0; i<Div+1; i++){
		MoveToEx(hdc, i * DivGap, 0, NULL);
		LineTo(hdc, i * DivGap, cy);
	}
	CellRect.right = DivGap;

	DivGap	= cy / Div;
	for(int i=0; i<Div+1; i++){
		MoveToEx(hdc, 0, i * DivGap, NULL);
		LineTo(hdc, cx, i * DivGap);
	}
	CellRect.bottom = DivGap;
	SelectObject(hdc, hOldPen);

	CellRect.left = CellRect.top = 0;

	RowGap		= CellRect.bottom - CellRect.top;
	ColumnGap	= CellRect.right - CellRect.left;

	// RED
	#define RUBYRED				0xE0115F
	#define ROSERED				0xFF033E
	#define CRIMSONRED			0xDC143C
	#define VERMILIONRED		0xE34234

	// BLUE
	#define DEEPSKYBLUE			0x00BFFF
	#define LIGHTSKYBLUE		0x87CEFA
	#define LIGHTBLUEMIST		0x9AC0CD
	#define LIGHTCYANBLUE		0xE0FFFF

	HBRUSH hTodayBrush				= CreateSolidBrush(RGB(224,255,255)),
		   hOldBrush;

	COLORREF BeginningWeekendColor	= RGB(0,191,255),
			 HolidayColor			= RGB(255,3,62);

	SetBkMode(hdc, TRANSPARENT);

	WCHAR DayOfTheWeekBuf[32];
	for(int i=0; i<Div; i++){
		if(i == 0){ SetTextColor(hdc, HolidayColor); }
		else if(i == 6){ SetTextColor(hdc, BeginningWeekendColor); }
		else { SetTextColor(hdc, RGB(0,0,0)); }
		StringCbPrintf(DayOfTheWeekBuf, sizeof(DayOfTheWeekBuf), L"%s", DayOfTheWeek[i]);
		GetTextExtentPoint32(hdc, DayOfTheWeekBuf, wcslen(DayOfTheWeekBuf), &TextSize);
		x = (CellRect.right - CellRect.left - TextSize.cx) / 2;
		y = (CellRect.bottom - CellRect.top - TextSize.cy) / 2;	

		TextOut(hdc, i * ColumnGap + x, CellRect.left + y, DayOfTheWeekBuf, wcslen(DayOfTheWeekBuf));
	}

	POINT Origin;
	WCHAR DayBuf[256];
	int yy				= CellRect.top + RowGap,
		iRowRadius		= RowGap / 2,
		iColumnRadius	= ColumnGap / 2;
	for(Day = 1; Day <= LastDay; Day++){
		StringCbPrintf(DayBuf, sizeof(DayBuf), L"%d", Day);
		GetTextExtentPoint32(hdc, DayBuf, wcslen(DayBuf), &TextSize);
		x = (CellRect.right - CellRect.left - TextSize.cx) / 2;
		y = (CellRect.bottom - CellRect.top - TextSize.cy) / 2;	

		if(Year == today.wYear && Month == today.wMonth && Day == today.wDay){
			hOldPen		= (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
			hOldBrush	= (HBRUSH)SelectObject(hdc, hTodayBrush);

			Origin.x	= DayOfWeek * ColumnGap + iColumnRadius;
			Origin.y	= yy + iRowRadius;

			Ellipse(hdc, Origin.x - iColumnRadius, Origin.y - iRowRadius, Origin.x + iColumnRadius, Origin.y + iRowRadius);
			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
		}

		if(DayOfWeek == 0){
			SetTextColor(hdc, HolidayColor);
		}else if(DayOfWeek == 6){
			SetTextColor(hdc, BeginningWeekendColor);
		}else{
			SetTextColor(hdc, RGB(0,0,0));
		}

		TextOut(hdc, DayOfWeek * ColumnGap + x, yy + y, DayBuf, wcslen(DayBuf));
	
		DayOfWeek++;
		if(DayOfWeek == 7){
			DayOfWeek = 0;
			yy += RowGap;
		}
	}

	DeleteObject(hTodayBrush);
	DeleteObject(hPen);

	/*
	// 범위 유효성 검사 - 캘린더 확장 기능 추가시 적용
	WCHAR	lpszDate[256];

	HWND hDateWnd = FindWindowEx(GetParent(_hWnd), NULL, L"MySubClassingEdit", NULL);
	while(IDC_EDDATE != (INT_PTR)GetDlgCtrlID(hDateWnd)){
		hDateWnd = FindWindowEx(GetParent(_hWnd), NULL, L"MySubClassingEdit", NULL);
	}

	GetWindowText(hDateWnd, lpszDate, 256);
	if(lpszDate == NULL){ MessageBeep(0); return 0;}

	int TemplateDateLength		= wcslen(TEMPLATE_DATE),
		TemplateDateUntilLength	= wcslen(TEMPLATE_DATE_UNTIL),
		TextLength				= wcslen(lpszDate);

	if(TextLength != TemplateDateLength && TextLength != TemplateDateUntilLength){
		MessageBeep(0);
		return 0;
	}

	// 포맷 검사
	for(int i=0; lpszDate[i]; i++){
		if(lpszDate[i] >= '0' && lpszDate[i] <'9'){
			lpszDate[i] = '*';
		}
	}

	if(wcscmp(lpszDate, TEMPLATE_DATE) != 0 && wcscmp(lpszDate, TEMPLATE_DATE_UNTIL) != 0){
		MessageBeep(0);
		return 0;
	}
	
	const int nSeps = 3;
	int i		= 0,
		j		= 0,
		x		= 0,
		DateTokenInt[nSeps] = {0,};

	WCHAR DateToken[nSeps][256];
	WCHAR Separators[] = L"-";					// 추후 '.'도 추가될 수 있음
	WCHAR* ptr = wcstok(lpszDate, lpszDate);
	while(ptr != NULL){
		StringCbCopy(DateToken[i], sizeof(DateToken[i]), ptr);
		ptr = wcstok(NULL, Separators);
	}

	for(i=0; nSeps; i++){
		x = 0;
		while(DateToken[i][j]){
			x*= 10;
			x+= DateToken[i][j] - '0';

			j++;
		}

		DateTokenInt[i] = x;
	}

	int Year	= 0,
		Month	= 1,
		Day		= 2;
	if(DateToken[Year] ... // 범위 검사)
	*/
}

void PopupWindow::DrawMouseTracker(HDC hdc){
	POINT Mouse;
	GetCursorPos(&Mouse);
	ScreenToClient(_hWnd, &Mouse);

	RECT crt;
	GetClientRect(_hWnd, &crt);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);

	HBRUSH hOldBrush;
	Mouse.y -= (crt.bottom - crt.top) - bmp.bmHeight;
	hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Ellipse(hdc, Mouse.x - 5, Mouse.y - 5, Mouse. x + 5, Mouse.y + 5);
	SelectObject(hdc, hOldBrush);
}


