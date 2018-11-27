// MaskDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "rfConsole.h"
#include "MaskDlg.h"
#include "afxdialogex.h"


// CMaskDlg dialog

IMPLEMENT_DYNAMIC(CMaskDlg, CDialog)

CMaskDlg::CMaskDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMaskDlg::IDD, pParent)
{

}

CMaskDlg::~CMaskDlg()
{
}

void CMaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMaskDlg, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMaskDlg message handlers
BOOL CMaskDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here	
	InitializeComponent();

	return TRUE; 
}

void CMaskDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

void CMaskDlg::InitializeComponent()
{

}