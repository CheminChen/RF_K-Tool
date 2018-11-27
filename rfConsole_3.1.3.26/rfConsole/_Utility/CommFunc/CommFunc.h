#pragma once

#include "..\..\..\..\Lib\rfDefine.h"

class CCommFunc
{
public:
	CCommFunc(void);
	~CCommFunc(void);

public:
	static void GetLocalPath();
	static bool GetFileVersion(TCHAR* pPath, TCHAR* pRespose, int nResposeLens=MAX_PATH);


	static bool MakeDir(TCHAR *pPath);
	static bool FileExist(TCHAR *pFilePath);
	
	static void StringTrim(TCHAR *pString);
	static int FindLastOf(TCHAR *pStringBuffer, TCHAR *pKey);

	static void OutputTracelog(TCHAR *szFormat, ...);

	static CString m_ModulePath;
	static CString m_LogPath;
	static CString m_ModuleName;

	//vERSION Info
	static TCHAR m_ModuleVersion[MAX_PATH];
	static TCHAR m_IQMeasureVersion[MAX_PATH];
	static TCHAR m_vDUTversion[MAX_PATH];
};