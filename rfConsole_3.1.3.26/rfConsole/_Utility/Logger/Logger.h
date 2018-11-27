#pragma once
#include "..\..\..\..\Lib\rfDefine.h"
#include "..\CommFunc\CommFunc.h"

class CLogger
{
public:
	CLogger(void);
	~CLogger(void);

public:
	void StartLogger(bool bShowDetail, LPSTR lpFile);
	void StartLogger();
	void StopLogger();

	void OutputLog(LOGTYPE logType, WORD wColor, TCHAR *szFormat, ...);
	void OutputCsv(LPSTR lpType, LPSTR szTimestamp, LPSTR szMac, LPSTR szSN, CStringArray *ptitleArray, CStringArray *pvalueArray);
	void OutputCsv(LPSTR lpType, CString strData);

	void OutputTracelog(TCHAR *szFormat, ...);

private:
	void printColorString(WORD wColor, LPSTR lpszMsg);
	

	bool			m_bShowDetail;
	HANDLE			m_hFile;
	TCHAR			m_LogFile[MAX_PATH];
};

