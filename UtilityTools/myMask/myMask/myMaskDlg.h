
// myMaskDlg.h : header file
//

#pragma once
#include "..\..\..\lib\rfDefine.h"
#include "..\..\..\Lib\CommClass\SysTray\SysTray.h"

#define WM_SYSTRAY	WM_USER+999

// CmyMaskDlg dialog
class CmyMaskDlg : public CDialogEx
{
// Construction
public:
	CmyMaskDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MYMASK_DIALOG };
	CWinFormsControl<ZedGraph::ZedGraphControl>	m_ZedGraph;
	CSysTray			m_SysTray;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedCheckSave();
	afx_msg LRESULT OnUpdateSMData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnMenuAlwaysOnTop();
	afx_msg void OnMenuExit();
	DECLARE_MESSAGE_MAP()

private:
	void DrawMask(LPSTR lpTestItem, int fftLength, double *freqsHz, double *SpectrumMask, double *tracedbm);
	
	BOOL			m_bAutoSave, m_bExit, m_bAlwaysOnTop;
	
};
