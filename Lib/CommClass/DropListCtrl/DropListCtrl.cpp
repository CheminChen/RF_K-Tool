// DropListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "DropListCtrl.h"


// CDropListCtrl

IMPLEMENT_DYNAMIC(CDropListCtrl, CListCtrl)

CDropListCtrl::CDropListCtrl()
{

}

CDropListCtrl::~CDropListCtrl()
{
}

BEGIN_MESSAGE_MAP(CDropListCtrl, CListCtrl)
	ON_WM_DROPFILES()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CDropListCtrl message handlers

int CDropListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if(CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	DragAcceptFiles(TRUE);
	return 0;
}

BOOL CDropListCtrl::SetCallBackFunction(LPFN_DROP_FILES_CALLBACK DropCallback)
{
	if(DropCallback)
	{
		m_DropCallback = DropCallback;
		return TRUE;
	}

	return FALSE;
}

void CDropListCtrl::OnDropFiles(HDROP dropInfo)
{
	UINT nNumFilesDropped = DragQueryFile(dropInfo, 0xFFFFFFFF, NULL, 0);
	
	TCHAR szDrop[MAX_PATH+1];
	SHFILEINFO sfi;
	struct _stat buf;
	BOOL bFlag;

	for(UINT nFile=0 ; nFile<nNumFilesDropped ; nFile++)
	{
		DragQueryFile(dropInfo, nFile, szDrop, MAX_PATH+1);

		_stat(szDrop, &buf);
		SHGetFileInfo((LPCTSTR)szDrop, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

		if((buf.st_mode & _S_IFDIR) == _S_IFDIR) bFlag = AnalysisDropFile(DROP_FOLDER, sfi.iIcon, szDrop);
		else bFlag = AnalysisDropFile(DROP_FILE, sfi.iIcon, szDrop);

		if(bFlag) m_DropCallback(this, szDrop, sfi.iIcon);
	}
	
	DragFinish(dropInfo);
}

BOOL CDropListCtrl::AnalysisDropFile(DROPTYPE dType, int nImage, CString strDropPath)
{
	CString strTarFolder, strTarFile;
	
	for(int n=0 ; n<m_FileArray.GetSize() ; n++)
	{
		FILEITEM *pItem = m_FileArray.GetAt(n);
		if(strDropPath.Compare(pItem->DropPath) == 0) return FALSE;
	}

	
	FILEITEM *pItem;
	if(dType == DROP_FILE) 
	{
		strTarFolder = strDropPath.Mid(0, strDropPath.ReverseFind(_T('\\'))+1);
		strTarFile = strDropPath.Mid(strDropPath.ReverseFind(_T('\\'))+1);

		pItem = new FILEITEM;
		memset(pItem, 0, sizeof(FILEITEM));
		strcpy_s(pItem->DropPath, MAX_PATH, strDropPath.GetBuffer());			
		strcpy_s(pItem->TarFolder, MAX_PATH, strTarFolder.GetBuffer());
		strcpy_s(pItem->TarFile, MAX_PATH, strTarFile.GetBuffer());
		pItem->Icon = nImage;	

		m_FileArray.Add(pItem);

	}else
	{
		CString strDir;
		CFileFind finder;
		BOOL bFlag = TRUE;

		strDir.Format(_T("%s\\*.*"), strDropPath);
		if(finder.FindFile(strDir))
		{
			int nFileCnt = 0;
			while(bFlag)
			{
				bFlag = finder.FindNextFile();
				if(finder.IsDots() || finder.IsDirectory()) continue;

				CString strFile = finder.GetFilePath();
				CString strTmp = strDropPath.Mid(0, strFile.ReverseFind(_T('\\'))-1);
				int nFind = strTmp.ReverseFind(_T('\\'))+1;
				strTarFolder = strFile.Mid(0, nFind);
				strTarFile = strFile.Mid(nFind);

				pItem = new FILEITEM;
				memset(pItem, 0, sizeof(FILEITEM));
				strcpy_s(pItem->DropPath, MAX_PATH, strDropPath.GetBuffer());			
				strcpy_s(pItem->TarFolder, MAX_PATH, strTarFolder.GetBuffer());
				strcpy_s(pItem->TarFile, MAX_PATH, strTarFile.GetBuffer());
				pItem->Icon = nImage;	

				m_FileArray.Add(pItem);
				nFileCnt++;
			}

			if(nFileCnt == 0)
			{
				CString strMsg;
				strMsg.Format(_T("The folder '%s' doesn't include any file\nPlease check again."), strDropPath);
				AfxMessageBox(strMsg, MB_OK | MB_ICONSTOP);

				return FALSE;
			}
		}		
	}
	
	return TRUE;
}

void CDropListCtrl::DeleteAllFileItem()
{
	while(m_FileArray.GetSize())
	{
		delete m_FileArray.GetAt(0);
		m_FileArray.RemoveAt(0);
	}
}

BOOL CDropListCtrl::DeleteFileItem(CString strPath)
{
	for(int n=0 ; n<m_FileArray.GetSize() ; n++)
	{
		FILEITEM *pItem = m_FileArray.GetAt(n);

		if(strstr(pItem->DropPath, strPath.GetBuffer()))
		{
			delete pItem;
			m_FileArray.RemoveAt(n);
			n = -1; 
			continue;
		}
	}

	return TRUE;
}