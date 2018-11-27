#pragma once
class CWndFunc
{
public:
	CWndFunc(void);
	~CWndFunc(void);

public:
	static void CreateToolTips(HWND hParentWnd, unsigned int nUID, LPTSTR lpszText);
	static void FlashTheComponent(HWND hWnd, DWORD dwNoticePeriod);

	static void DrawComponentBorder(CWnd* pWnd);
	static HWND m_hWndToolTip;
};

