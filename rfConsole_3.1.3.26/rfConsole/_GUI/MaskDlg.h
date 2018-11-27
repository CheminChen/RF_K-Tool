#pragma once
#include "..\Resource.h"

sing namespace ZedGraph;
	using namespace System::Drawing;

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


// CMaskDlg dialog

class CMaskDlg : public CDialog
{
	DECLARE_DYNAMIC(CMaskDlg)

public:
	CMaskDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMaskDlg();

// Dialog Data
	enum { IDD = IDD_MASKDLG };

protected:
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void InitializeComponent();
};
