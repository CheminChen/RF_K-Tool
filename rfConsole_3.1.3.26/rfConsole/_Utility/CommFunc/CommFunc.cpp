#include "StdAfx.h"
#include "CommFunc.h"


CString CCommFunc::m_ModulePath;
CString CCommFunc::m_LogPath;
CString CCommFunc::m_ModuleName;
TCHAR CCommFunc::m_ModuleVersion[MAX_PATH];
TCHAR CCommFunc::m_IQMeasureVersion[MAX_PATH];
TCHAR CCommFunc::m_vDUTversion[MAX_PATH];

CCommFunc::CCommFunc(void)
{
	memset(m_ModuleVersion, 0, MAX_PATH);
}

CCommFunc::~CCommFunc(void)
{
}

bool CCommFunc::FileExist(TCHAR *pFilePath)
{
	if(FILE *pFile = fopen(pFilePath, _T("r")))
	{
		fclose(pFile);
		return true;
	}
	return false;
}

void CCommFunc::OutputTracelog(TCHAR *szFormat, ...)
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

int CCommFunc::FindLastOf(TCHAR *pStringBuffer, TCHAR *pKey)
{
	int nIdx = -1;
	TCHAR *pTmpString = pStringBuffer, *pIdx;

	while((pIdx=strstr(pTmpString, pKey)))
	{
		pTmpString += strlen(pKey);
		nIdx = pIdx - &pStringBuffer[0];
	}

	return nIdx;
}

void CCommFunc::StringTrim(TCHAR *pString)
{
	TCHAR *pCh = NULL, *pEnd, *pStart = NULL;
	int nIdx = -1;

	if((pCh = strstr(pString, _T("//"))))
	{
		nIdx = pCh-&pString[0];
		pString[nIdx] = _T('\0');		
	}

	//Remove the space
	pStart = &pString[0];
	pEnd = pStart + strlen(pString)-1;
	while(pEnd > pStart && isspace((unsigned char)*pEnd)) pEnd--;
	*(pEnd+1) = _T('\0');
}

void CCommFunc::GetLocalPath()
{
	CString strTmp;
	TCHAR *pPath = new TCHAR[MAX_PATH];

	if(GetModuleFileName(NULL, pPath, MAX_PATH) == 0)
	{
		CCommFunc::OutputTracelog(_T("Fail to get the Module path, errCode: %d"), GetLastError());
		goto go_End;
	}

	strTmp.Format(_T("%s"), pPath);
	int iPos = strTmp.ReverseFind('\\');
	m_ModulePath = strTmp.Mid(0, iPos+1);
	
	m_LogPath = m_ModulePath+_T("Log\\");
	CCommFunc::MakeDir(m_LogPath.GetBuffer());

	m_ModuleName = strTmp.Mid(iPos+1, strTmp.GetLength()-(iPos+1)-4);

	CCommFunc::GetFileVersion((m_ModulePath+_T("\\")+m_ModuleName+_T(".EXE")).GetBuffer(), m_ModuleVersion);
	CCommFunc::GetFileVersion((m_ModulePath+_T("\\")+_T("IQmeasure.dll")).GetBuffer(), m_IQMeasureVersion);
	CCommFunc::GetFileVersion((m_ModulePath+_T("\\")+_T("vDUTctrl.dll")).GetBuffer(), m_vDUTversion);
go_End:
	DEL_ARRAY(pPath)
}

bool CCommFunc::GetFileVersion(TCHAR* pPath, TCHAR* pRespose, int nResposeLens)
{
	bool bRtn = false;
	DWORD dwSize = 0, dwHandle = 0;
	TCHAR *pVersion = NULL;
	void *pBuffer = NULL;
	UINT uLen = 0;

	if((dwSize=GetFileVersionInfoSize(pPath, &dwHandle)) == 0)
	{
		CCommFunc::OutputTracelog(_T("Fail to get the size of version info, errCode: %d"), GetLastError());
		goto go_End;
	}

	pVersion = new TCHAR[dwSize];
	if(!(bRtn=GetFileVersionInfo(pPath, dwHandle, dwSize, pVersion)))
	{
		CCommFunc::OutputTracelog(_T("Fail to get the version info, errCode: %d"), GetLastError());
		goto go_End;
	}

	if(!(bRtn=VerQueryValue(pVersion, _T("\\"), &pBuffer, &uLen)))
	{
		CCommFunc::OutputTracelog(_T("Fail to query the value of version, errCode: %d"), GetLastError());
		goto go_End;
	}

	VS_FIXEDFILEINFO vsf;
	memcpy(&vsf, pBuffer, sizeof(VS_FIXEDFILEINFO));
	sprintf_s(pRespose, nResposeLens, _T("%u.%u.%u.%u"), HIWORD(vsf.dwFileVersionMS), LOWORD(vsf.dwFileVersionMS), HIWORD(vsf.dwFileVersionLS), LOWORD(vsf.dwFileVersionLS));	

go_End:
	DEL_ARRAY(pVersion)
	return bRtn;
}

bool CCommFunc::MakeDir(TCHAR *pPath)
{
	return ((SHCreateDirectoryEx(NULL, pPath, NULL) == ERROR_SUCCESS) || (GetLastError() == ERROR_ALREADY_EXISTS));
}

