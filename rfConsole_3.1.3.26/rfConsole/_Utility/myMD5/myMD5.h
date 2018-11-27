#pragma once

#include "..\CommFunc\CommFunc.h"
#include "..\Logger\Logger.h"

#include "..\..\..\..\Lib\rfDefine.h"

typedef struct {
	unsigned char digest[16];
	unsigned long hHash;
} MD5Context;

class myMD5
{
public:
	myMD5(void);
	~myMD5(void);

public:
	bool calculateMD5(LPSTR pszFile, LPSTR pszMD5, int nLens=MD5_LENGTH);

	CLogger					*m_pLogger;

private:
	static UINT MD5ThreadProc(LPVOID pParam);
	bool CryptStartup();
	void CryptCleanup();
	void MD5Init(MD5Context *ctx);
	void MD5Update(MD5Context *ctx, unsigned char const *pBuf, unsigned int Lens);
	void MD5Final(MD5Context *ctx);
	

	HANDLE					m_hThreadFinish;
	bool					m_bRunning;

	LPSTR					m_pszFile;
	TCHAR					m_md5Value[STRING_LENS_256];

	HCRYPTPROV				m_hCryptProv;
};

