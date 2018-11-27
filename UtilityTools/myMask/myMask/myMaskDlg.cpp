
// myMaskDlg.cpp : implementation file
//

#include "stdafx.h"
#include "myMask.h"
#include "myMaskDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmyMaskDlg dialog

using namespace System;
using namespace System::Drawing;
using namespace System::Collections;
using namespace ZedGraph;

CmyMaskDlg::CmyMaskDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmyMaskDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bAutoSave = FALSE;

	m_bExit = FALSE;
	m_bAlwaysOnTop = TRUE;
}

void CmyMaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_ManagedControl(pDX, IDC_ZEDGRAPH, m_ZedGraph);
}

BEGIN_MESSAGE_MAP(CmyMaskDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SPECTRUM_MASK, &CmyMaskDlg::OnUpdateSMData)
	ON_MESSAGE(WM_COPYDATA, &CmyMaskDlg::OnCopyData)
	ON_BN_CLICKED(IDC_CHECK_SAVE, &CmyMaskDlg::OnBnClickedCheckSave)
	ON_WM_CLOSE()
	ON_COMMAND(ID_SYSTRAY_ALWAYSONTOP, OnMenuAlwaysOnTop)
	ON_COMMAND(ID_SYSTRAY_EXIT, OnMenuExit)
END_MESSAGE_MAP()


// CmyMaskDlg message handlers

BOOL CmyMaskDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_ZedGraph->GraphPane->Title = "Spectrum Mask";
	m_ZedGraph->GraphPane->XAxis->Title = "Frequency [MHz]";
	m_ZedGraph->GraphPane->YAxis->Title = "Power spectral density";

	CString strTmp = _T("Spectrum Mask");
	m_SysTray.Create(m_hIcon, strTmp, 1, WM_SYSTRAY, IDR_MENU_SYSTRAY);
	m_SysTray.RegistryNotify(this->m_hWnd);
	m_SysTray.SetCheckStatus(ID_SYSTRAY_ALWAYSONTOP, STATUS_ENABLE, FALSE);
	//On Top
	m_SysTray.SetCheckStatus(ID_SYSTRAY_ALWAYSONTOP, STATUS_ENABLE, TRUE);
	m_SysTray.SetCheckStatus(ID_SYSTRAY_ALWAYSONTOP, STATUS_CHECK, m_bAlwaysOnTop);
	::SetWindowPos(this->m_hWnd, m_bAlwaysOnTop?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CmyMaskDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CmyMaskDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CmyMaskDlg::OnUpdateSMData(WPARAM wParam, LPARAM lParam)
{
	LPSTR Test = (LPSTR)wParam;

	return TRUE;
}

#define LP_BUFFER_SIZE	8192 /*Reference from LPIQ.h*/
LRESULT CmyMaskDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT*)lParam;

	if(pCopyData)
	{
		TCHAR *pBuffer = new TCHAR[pCopyData->cbData];
		memcpy_s(pBuffer, pCopyData->cbData, pCopyData->lpData, pCopyData->cbData);

		LPSTR lpTestItem = NULL;
		int *bufferSizeY;
		double *bufferRealX = NULL, *bufferRealY=NULL;
		double *maskWiFi = NULL;

		lpTestItem = (LPSTR)&pBuffer[0];
		bufferSizeY = (int*)&pBuffer[sizeof(TCHAR)*STRING_LENS_64];
		bufferRealX = (double*)&pBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)];
		maskWiFi = (double*)&pBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)+(LP_BUFFER_SIZE*8)*sizeof(double)];
		bufferRealY = (double*)&pBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)+(LP_BUFFER_SIZE*8)*sizeof(double)+*bufferSizeY*sizeof(double)];
		
		DrawMask(lpTestItem, *bufferSizeY, bufferRealX, maskWiFi, bufferRealY);

		if(pBuffer) delete[] pBuffer;
	}
	return TRUE;
}

void CmyMaskDlg::DrawMask(LPSTR lpTestItem, int fftLength, double *freqsHz, double *SpectrumMask, double *tracedbm)
{
	PointPairList^ mask = gcnew PointPairList(); 
	PointPairList^ value = gcnew PointPairList(); 

	for (int i = 0; i < fftLength; i++)
	{
		mask->Add(freqsHz[i]*1e-6, SpectrumMask[i]);
		value->Add(freqsHz[i]*1e-6, tracedbm[i]);
	}
	
	m_ZedGraph->GraphPane->CurveList->Clear();

	//myPane.Title.Text
	m_ZedGraph->GraphPane->Title = gcnew System::String(lpTestItem);	

	if(fftLength>0)
	{
		m_ZedGraph->GraphPane->AddCurve("", mask, Color::Red, SymbolType::None);
		m_ZedGraph->GraphPane->AddCurve("", value, Color::Green, SymbolType::None);
	}

	m_ZedGraph->AxisChange();
	m_ZedGraph->Refresh();

	if(m_bAutoSave)
	{
		//m_ZedGraph->MasterPane->GetImage().Save()
	}
}

void CmyMaskDlg::OnBnClickedCheckSave()
{
	m_bAutoSave = ((CButton*)GetDlgItem(IDC_CHECK_SAVE))->GetCheck();
}

void CmyMaskDlg::OnMenuAlwaysOnTop()
{
	m_bAlwaysOnTop = !m_bAlwaysOnTop;
	m_SysTray.SetCheckStatus(ID_SYSTRAY_ALWAYSONTOP, STATUS_ENABLE, TRUE);
	m_SysTray.SetCheckStatus(ID_SYSTRAY_ALWAYSONTOP, STATUS_CHECK, m_bAlwaysOnTop);

	::SetWindowPos(this->m_hWnd, m_bAlwaysOnTop?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);	
}

void CmyMaskDlg::OnMenuExit()
{
	m_bExit = true;
	SendMessage(WM_CLOSE);
}

void CmyMaskDlg::OnClose()
{
	if(!m_bExit) this->ShowWindow(SW_HIDE);
	else CDialogEx::OnClose();
}
