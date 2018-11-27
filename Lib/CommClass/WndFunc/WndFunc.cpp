#include "StdAfx.h"
#include "WndFunc.h"

HWND CWndFunc::m_hWndToolTip;

CWndFunc::CWndFunc(void)
{
}

CWndFunc::~CWndFunc(void)
{
}

void CWndFunc::CreateToolTips(HWND hParentWnd, unsigned int nUID, LPTSTR lpszText)
{
	RECT rect;
	TOOLINFO ti;
	INITCOMMONCONTROLSEX iccex;
	iccex.dwICC = ICC_WIN95_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&iccex);

	if(m_hWndToolTip) DestroyWindow(m_hWndToolTip);
	m_hWndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP, CW_USEDEFAULT, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParentWnd, NULL, (HINSTANCE)GetWindowLong(hParentWnd, GWL_HINSTANCE), NULL);

	::SetWindowPos(m_hWndToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

	::GetClientRect(hParentWnd, &rect);	
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hParentWnd;
	ti.hinst = (HINSTANCE)GetWindowLong(hParentWnd, GWL_HINSTANCE);
	ti.uId = nUID;
	ti.lpszText = lpszText;
	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	::SendMessage(m_hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
	
}

void CWndFunc::FlashTheComponent(HWND hWnd, DWORD dwNoticePeriod)
{
	double dbTimeStart = GetTickCount();

	do 
	{
		DrawComponentBorder(CWnd::FromHandle(hWnd));
		Sleep(100);

	} while ((GetTickCount() - dbTimeStart) < dwNoticePeriod);
}

void CWndFunc::DrawComponentBorder(CWnd* pWnd)
{
	if(pWnd == NULL) return;

	CRect WndRect;
	pWnd->GetWindowRect(&WndRect);
	CWindowDC WndDC(pWnd);	

	CPen pen;
	pen.CreatePen(PS_INSIDEFRAME, 3 * GetSystemMetrics(SM_CXBORDER), RGB(0, 0, 0));

	int dcSave = WndDC.SaveDC();
	WndDC.SetROP2(R2_NOT);

	WndDC.SelectObject(&pen);
	WndDC.SelectObject(GetStockObject(NULL_BRUSH));
	WndDC.Rectangle(0, 0, WndRect.Width(), WndRect.Height());
	WndDC.RestoreDC(dcSave);
}