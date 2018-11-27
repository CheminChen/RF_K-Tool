#include "StdAfx.h"
#include "Logger.h"


CLogger::CLogger(void)
{
	memset(m_LogFile, 0, MAX_PATH);
	m_hFile = NULL;
}

CLogger::~CLogger(void)
{
	StopLogger();
}

void CLogger::OutputTracelog(TCHAR *szFormat, ...)
{
#if (_DEBUG)
	va_list marker;
	TCHAR *pszBuffer = new TCHAR[MAX_ARCBUFFER_SIZE];
	memset(pszBuffer, _T('\0'), MAX_ARCBUFFER_SIZE);

	va_start(marker, szFormat);
	vsprintf_s(pszBuffer, MAX_ARCBUFFER_SIZE, szFormat, marker);
	va_end(marker);

	OutputDebugString(pszBuffer);
	DEL_ARRAY(pszBuffer);
#endif
}

void CLogger::StartLogger(bool bShowDetail, LPSTR lpFile)
{
	TCHAR *pszLine = new TCHAR[STRING_LENS_256], *pszNewFile = new TCHAR[MAX_PATH], *pTimestamp = new TCHAR[STRING_LENS_256];
	DWORD dwWriteByte;
	ifstream fStream(lpFile);
	CTime cTime = CTime::GetCurrentTime();
	
	m_bShowDetail = bShowDetail;
	strcpy_s(m_LogFile, MAX_PATH, lpFile);

	/*Read timestamp*/
	memset(pszLine, _T('\0'), STRING_LENS_256);
	memset(pTimestamp, _T('\0'), STRING_LENS_256);
	memset(pszNewFile, _T('\0'), MAX_PATH);
	if(CCommFunc::FileExist(lpFile))
	{		
		fStream.getline(pszLine, STRING_LENS_256);
		fStream.close();

		if(strlen(pszLine)) sprintf_s(pszNewFile, MAX_PATH, _T("%s%s_Tracelog.log"), CCommFunc::m_LogPath, pszLine);
		else sprintf_s(pszNewFile, MAX_PATH, _T("%s_Tracelog.log"), CCommFunc::m_LogPath);
		
		if(!CopyFile(lpFile, pszNewFile, FALSE)) 
		{
			OutputTracelog(_T("CopyFile failed, errCode: %d"), GetLastError());
			goto go_End;
		}else
		{			
			if(!DeleteFile(lpFile)) 
			{
				OutputTracelog(_T("DeleteFile failed, errCode: %d"), GetLastError());
				goto go_End;			
			}
		}
	}

	/*Create file handle*/
	if(m_hFile == NULL)
	{
		if((m_hFile=::CreateFile(lpFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		{
			OutputTracelog(_T("Fail to create the file handle, errCode: %d"), GetLastError());
			goto go_End;
		}

		sprintf_s(pTimestamp, STRING_LENS_256, _T("%s\r\n\r\n"), cTime.Format(_T("%m-%d_%H-%M-%S")));

		if(::SetFilePointer(m_hFile, 0, NULL, FILE_END) != INVALID_SET_FILE_POINTER)
		{
			if(!(::WriteFile(m_hFile, pTimestamp, (DWORD)(strlen(pTimestamp)), &dwWriteByte, NULL)))
			{
				OutputTracelog(_T("WriteFile failed, errCode: %d"), GetLastError());
			}
		}else OutputTracelog(_T("SetFilePointer failed, errCode: %d"), GetLastError());
	}
		
go_End:
	DEL_ARRAY(pszLine)
	DEL_ARRAY(pszNewFile)
	DEL_ARRAY(pTimestamp)
}

/*For the second round, Not for loop*/
void CLogger::StartLogger()
{
	if(CCommFunc::FileExist(m_LogFile) && (m_hFile == NULL))
	{
		if((m_hFile=::CreateFile(m_LogFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		{
			OutputTracelog(_T("Fail to create the file handle, errCode: %d"), GetLastError());
		}
	}
}

void CLogger::StopLogger()
{
	if(m_hFile) 
	{
		::CloseHandle(m_hFile);
		m_hFile = NULL;
	}
}

void CLogger::OutputCsv(LPSTR lpType, CString strData)
{
	HANDLE hFile = NULL;
	CString strFile;
	DWORD dwWriteByte;

	strFile.Format(_T("%s%s_TestResult.csv"), CCommFunc::m_LogPath, lpType);

	if((hFile = ::CreateFile(strFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) 
	{
		OutputTracelog(_T("CreateFile failed, errCode: %d"), GetLastError());
		goto go_End;
	}

	if((::SetFilePointer(hFile, 0, NULL, FILE_END)) == INVALID_SET_FILE_POINTER) 
	{
		OutputTracelog(_T("SetFilePointer failed, errCode: %d"), GetLastError());
		goto go_End;
	}

	strData += _T("\r\n");
	::WriteFile(hFile, strData.GetBuffer(), (DWORD)(strData.GetLength()), &dwWriteByte, NULL);

go_End:
	if(hFile) ::CloseHandle(hFile);
}

void CLogger::OutputCsv(LPSTR lpType, LPSTR szTimestamp, LPSTR szMac, LPSTR szSN, CStringArray *ptitleArray, CStringArray *pvalueArray)
{
	HANDLE hFile = NULL;
	CString strFile, strTmp = _T("");
	DWORD dwWriteByte;
	bool bFileExist = false;	
	
	if((ptitleArray->GetSize() == 0) || (pvalueArray->GetSize() == 0))
	{
		OutputTracelog(_T("OutputCsv failed, errMsg: no any title or value in the array"));
		goto go_End;
	}

	strFile.Format(_T("%s%s_TestResult.csv"), CCommFunc::m_LogPath, lpType);
	bFileExist = CCommFunc::FileExist(strFile.GetBuffer());

	if((hFile = ::CreateFile(strFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		OutputTracelog(_T("CreateFile failed, errCode: %d"), GetLastError());
		goto go_End;
	}

	if((::SetFilePointer(hFile, 0, NULL, FILE_END)) == INVALID_SET_FILE_POINTER) 
	{
		OutputTracelog(_T("SetFilePointer failed, errCode: %d"), GetLastError());
		goto go_End;
	}

	if(!bFileExist)
	{
		for(int n=0 ; n<ptitleArray->GetSize() ; n++) strTmp += (ptitleArray->GetAt(n) + _T(","));
		strTmp += _T("\r\n");
		::WriteFile(hFile, strTmp.GetBuffer(), (DWORD)(strTmp.GetLength()), &dwWriteByte, NULL);
	}

	strTmp.Format(_T("%s,%s,%s,"), szTimestamp, szMac, szSN);
	for(int n=0 ; n<pvalueArray->GetSize() ; n++) strTmp += (pvalueArray->GetAt(n) + _T(","));
	strTmp += _T("\r\n");
	::WriteFile(hFile, strTmp.GetBuffer(), (DWORD)(strTmp.GetLength()), &dwWriteByte, NULL);

go_End:
	if(hFile) ::CloseHandle(hFile);
}

void CLogger::OutputLog(LOGTYPE logType, WORD wColor, TCHAR *szFormat, ...)
{
	va_list marker;
	CTime cTime;
	DWORD dwWriteByte;
	CString strLog;
	TCHAR *pszBuffer = new TCHAR[MAX_ARCBUFFER_SIZE];
	memset(pszBuffer, 0, MAX_ARCBUFFER_SIZE);

	if((!m_bShowDetail) && (logType == TYPE_DETAILS)) goto go_End;

	va_start(marker, szFormat);
	vsprintf_s(pszBuffer, MAX_ARCBUFFER_SIZE, szFormat, marker);
	va_end(marker);

	strLog.Format(_T("    %s"), pszBuffer);	
	
	if(logType == TYPE_TITLE)
	{
		strLog += _T("\r\n");
		printColorString(COLOR_YELLOW, strLog.GetBuffer());
	}else if(logType == TYPE_SUMMARY) 
	{
		strLog += _T("\r\n");
		printColorString(COLOR_MAGENTA, strLog.GetBuffer());
	}else
	{
		strLog.Replace(_T("\r\n"), _T("\r\n    "));
		strLog += _T("\r\n");

		if(logType != TYPE_DETAILS) printColorString(wColor, strLog.GetBuffer());
	}

	/*Export to file*/
	if(m_hFile)
	{
		if((logType != TYPE_DETAILS) || (m_bShowDetail && (logType == TYPE_DETAILS)))
		{
			if(::SetFilePointer(m_hFile, 0, NULL, FILE_END) != INVALID_SET_FILE_POINTER)
			{
				if(!(::WriteFile(m_hFile, strLog.GetBuffer(), (DWORD)(strLog.GetLength()), &dwWriteByte, NULL)))
				{
					OutputTracelog(_T("WriteFile failed, errCode: %d"), GetLastError());
				}

#if (_DEBUG)
				CCommFunc::OutputTracelog(_T("%s)"), strLog.GetBuffer());
#endif
			}else OutputTracelog(_T("SetFilePointer failed, errCode: %d"), GetLastError());
		}		
	}else OutputTracelog(_T("m_hFile is NULL"));

go_End:
	DEL_ARRAY(pszBuffer);
}

void CLogger::printColorString(WORD wColor, LPSTR lpszMsg)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	WORD saved_attributes;

	/* Save current attributes */
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	saved_attributes = consoleInfo.wAttributes;

	SetConsoleTextAttribute(hConsole, wColor);
	printf(_T("%s"), lpszMsg);

	/* Restore original attributes */
	SetConsoleTextAttribute(hConsole, saved_attributes);
}