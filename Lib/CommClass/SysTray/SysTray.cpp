// SysTray.cpp : implementation file
//

#include "stdafx.h"
#include "SysTray.h"


// CSysTray

IMPLEMENT_DYNAMIC(CSysTray, CWnd)

CSysTray::CSysTray()
{
	ZeroMemory(&m_NotifyIconData, sizeof(m_NotifyIconData));
}

CSysTray::~CSysTray()
{
	while(m_CheckStatusAry.GetSize())
	{
		delete m_CheckStatusAry.GetAt(0);
		m_CheckStatusAry.RemoveAt(0);
	}

	m_NotifyIconData.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);

	DestroyWindow();
}

BEGIN_MESSAGE_MAP(CSysTray, CWnd)
END_MESSAGE_MAP()

// CSysTray message handlers

BOOL CSysTray::Create(HICON hIcon, LPCTSTR lpszToolTip, UINT uID, UINT uCBMsg, UINT uMenu)
{
	BOOL bRtn = FALSE;

	m_TrayMenu = uMenu;
	if(!CWnd::CreateEx(0, AfxRegisterWndClass(0), _T("CSystemTray Dummy Window"), WS_POPUP, 0,0,0,0, NULL, 0)) goto go_End;

	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIconData.hWnd = m_hWnd;
	m_NotifyIconData.uID = uID;
	m_NotifyIconData.hIcon = hIcon;
	m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_NotifyIconData.uCallbackMessage = uCBMsg;
	lstrcpy(m_NotifyIconData.szTip, lpszToolTip);
	bRtn = Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);

go_End:
	return bRtn;
}

void CSysTray::RegistryNotify(HWND hWnd)
{
	m_hPWnd = hWnd;
}

BOOL CSysTray::GetCheckStatus(UINT MenuItem, TYPE ItemType)
{
	MENUCHECK *pItem = NULL;

	for(int n=0 ; n<m_CheckStatusAry.GetSize() ; n++)
	{
		pItem = m_CheckStatusAry.GetAt(n);

		if(pItem->MenuItem == MenuItem)
		{
			if(ItemType == STATUS_CHECK) return pItem->bCheck;
			else if(ItemType == STATUS_ENABLE) return pItem->bEnable;
		}
	}
	return FALSE;
}

void CSysTray::SetCheckStatus(UINT MenuItem, TYPE ItemType, BOOL bFlag)
{
	MENUCHECK *pItem = NULL;

	for(int n=0 ; n<m_CheckStatusAry.GetSize() ; n++)
	{
		pItem = m_CheckStatusAry.GetAt(n);

		if(pItem->MenuItem == MenuItem)
		{
			if(ItemType == STATUS_CHECK) pItem->bCheck = bFlag;
			else if(ItemType == STATUS_ENABLE) pItem->bEnable = bFlag;

			return;
		}		
	}

	pItem = new MENUCHECK;
	memset(pItem, 0, sizeof(MENUCHECK));
	pItem->MenuItem = MenuItem;
	if(ItemType == STATUS_CHECK) pItem->bCheck = bFlag;
	else if(ItemType == STATUS_ENABLE) pItem->bEnable = bFlag;
	m_CheckStatusAry.Add(pItem);
}

BOOL CSysTray::SetToolTip(LPCTSTR lpszToolTip)
{
	m_NotifyIconData.uFlags = NIF_ICON|NIF_TIP;
	lstrcpy(m_NotifyIconData.szTip, lpszToolTip);

	return Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

BOOL CSysTray::SetIcon(HICON hIcon, LPCTSTR lpszToolTip)
{
	m_NotifyIconData.uFlags = NIF_ICON|NIF_TIP;
	m_NotifyIconData.hIcon = hIcon;
	lstrcpy(m_NotifyIconData.szTip, lpszToolTip);

	return Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

LRESULT CSysTray::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	if(uMsg == m_NotifyIconData.uCallbackMessage)
	{
		return OnTrayNotify(wParam, lParam);
	}

	return CWnd::WindowProc(uMsg, wParam, lParam);
}

LRESULT CSysTray::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	if(wParam != m_NotifyIconData.uID) return 0L;
	
	CWnd *pMainWnd = AfxGetMainWnd();

	switch(LOWORD(lParam))
	{
	case WM_RBUTTONUP:
		{
			if(m_TrayMenu != 0)
			{
				CMenu menu;
				VERIFY(menu.LoadMenu(m_TrayMenu));
				CMenu* pPopup = menu.GetSubMenu(0);

				POINT pt ;
				GetCursorPos (&pt) ;

				SetForegroundWindow();
				
				for(int n=0 ; n<pPopup->GetMenuItemCount() ; n++)
				{
					UINT PP = pPopup->GetMenuItemID(n);

					for(int nItem=0 ; nItem<m_CheckStatusAry.GetSize() ; nItem++)
					{
						UINT MenuItem = m_CheckStatusAry.GetAt(nItem)->MenuItem;
						
						if(pPopup->GetMenuItemID(n) == MenuItem)
						{							
							pPopup->CheckMenuItem(MenuItem, MF_BYCOMMAND | m_CheckStatusAry.GetAt(nItem)->bCheck ? MF_CHECKED : MF_UNCHECKED);
							pPopup->EnableMenuItem(MenuItem, m_CheckStatusAry.GetAt(nItem)->bEnable ? 0 : MF_DISABLED | MF_GRAYED);

							break;
						}
					}
				}
				
				//pPopup->CheckMenuItem(m_MenuItem, MF_BYPOSITION|MF_CHECKED);
				pPopup->TrackPopupMenu(TPM_LEFTALIGN |	TPM_RIGHTBUTTON, pt.x,  pt.y, AfxGetMainWnd());
			}			
		}
		break;
	case WM_LBUTTONDOWN:
		{
			//AfxMessageBox(_T("L button"));
			::ShowWindow(m_hPWnd, SW_SHOW);
		}
		break;
	default:
	break;
	}

	return 1L;
}

BOOL CSysTray::ShowBalloonMsg(PTSTR szBalloonTitle, PTSTR szBalloonMsg, DWORD dwIcon, UINT nTimeOut)
{
	BOOL bRtn = FALSE;

	if((!szBalloonTitle) || (strlen(szBalloonTitle) == 0) || (!szBalloonMsg) || (strlen(szBalloonMsg) == 0)) goto go_End;

	m_NotifyIconData.dwInfoFlags = dwIcon ;
	m_NotifyIconData.uFlags |= NIF_INFO;

	m_NotifyIconData.uTimeout = nTimeOut;
	strcpy_s(m_NotifyIconData.szInfoTitle, sizeof(m_NotifyIconData.szInfoTitle), szBalloonTitle);
	strcpy_s(m_NotifyIconData.szInfo, sizeof(m_NotifyIconData.szInfo), szBalloonMsg);

	bRtn = ::Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);


go_End:
	return bRtn;
}
