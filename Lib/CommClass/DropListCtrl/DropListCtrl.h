#pragma once
#include <sys/stat.h>

// CDropListCtrl
enum DROPTYPE
{
	DROP_FOLDER = 1,
	DROP_FILE = 2
};

typedef struct _TEXTITEM
{
	int nRow;
	int nColumn;
	COLORREF crText;
}TEXTITEM, *PTEXTITEM;
typedef CArray<TEXTITEM*, TEXTITEM*> ITEMCOLORARRAY;

typedef struct _FILEITEM
{
	TCHAR DropPath[MAX_PATH];
	TCHAR TarFolder[MAX_PATH];
	TCHAR TarFile[MAX_PATH];
	DROPTYPE dType;
	UINT Icon;
}FILEITEM, *PFILEITEM;
typedef CArray<FILEITEM*, FILEITEM*> FILEITEMARRAY;

typedef HRESULT (CALLBACK FAR * LPFN_DROP_FILES_CALLBACK)(CListCtrl*, CString, int);

class CDropListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CDropListCtrl)

public:
	CDropListCtrl();
	virtual ~CDropListCtrl();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP dropInfo);
	DECLARE_MESSAGE_MAP()

public:
	BOOL SetCallBackFunction(LPFN_DROP_FILES_CALLBACK DropCallback);
	BOOL DeleteFileItem(CString strPath);
	void DeleteAllFileItem();

	//CStringArray				m_FileList;
	FILEITEMARRAY				m_FileArray;

private:
	BOOL AnalysisDropFile(DROPTYPE dType, int nImage, CString strDropPath);

	ITEMCOLORARRAY				m_TextColorArray;
	LPFN_DROP_FILES_CALLBACK	m_DropCallback;
};


