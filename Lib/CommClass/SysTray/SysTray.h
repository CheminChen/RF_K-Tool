#pragma once

// CSysTray

enum TYPE {STATUS_CHECK=0, STATUS_ENABLE=1};

typedef struct _MENU_CHECK
{
	UINT MenuItem;
	BOOL bEnable;
	BOOL bCheck;
}MENUCHECK, *PMENUCHECK;
typedef CArray<MENUCHECK*, MENUCHECK*> STATUSARRAY;

class CSysTray : public CWnd
{
	DECLARE_DYNAMIC(CSysTray)

public:
	CSysTray();
	virtual ~CSysTray();

protected:
	DECLARE_MESSAGE_MAP()

public:
	BOOL Create(HICON hIcon, LPCTSTR lpszToolTip, UINT uID=1, UINT uCBMsg=WM_USER+999, UINT uMenu=0);
	void RegistryNotify(HWND hWnd);

	void SetCheckStatus(UINT MenuItem, TYPE ItemType, BOOL bFlag);
	BOOL GetCheckStatus(UINT MenuItem, TYPE ItemType);

	BOOL SetIcon(HICON hIcon, LPCTSTR lpszToolTip);
	BOOL SetToolTip(LPCTSTR lpszToolTip);
	
	BOOL ShowBalloonMsg(PTSTR szBalloonTitle, PTSTR szBalloonMsg, DWORD dwIcon, UINT nTimeOut);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);

private:
	UINT						m_TrayMenu;
	NOTIFYICONDATA				m_NotifyIconData;
	STATUSARRAY					m_CheckStatusAry;
	HWND						m_hPWnd;
};


