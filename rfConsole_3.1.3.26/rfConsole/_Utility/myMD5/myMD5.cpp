#include "StdAfx.h"
#include "myMD5.h"


myMD5::myMD5(void)
{
	m_bRunning = false;
	m_pszFile = NULL;
	memset(m_md5Value, 0, STRING_LENS_256);
}

myMD5::~myMD5(void)
{
}

#define MAX_STACK_SIZE 65535*100
bool myMD5::calculateMD5(LPSTR pszFile, LPSTR pszMD5, int nLens)
{
	bool bRtn = false;

	if(!m_bRunning)
	{
		m_pszFile = pszFile;

		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		m_hThreadFinish = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_bRunning = (AfxBeginThread(MD5ThreadProc, this, THREAD_PRIORITY_NORMAL, MAX_STACK_SIZE, 0, &sa) != NULL);
	}
	WaitForSingleObject(m_hThreadFinish, INFINITE);

	if(strlen(m_md5Value) <= nLens)
	{
		strcpy_s(pszMD5, nLens, m_md5Value);
		bRtn = true;
	}else m_pLogger->OutputTracelog(_T("Length overflow, expected length: %d, target length: %d"), strlen(m_md5Value), nLens);
	
	return bRtn;
}

UINT myMD5::MD5ThreadProc(LPVOID pParam)
{
	myMD5 *pMain = (myMD5*)pParam;
	DWORD dwRtn = 0;
	FILE *pInputFile = NULL;
	MD5Context md5Hash;
	unsigned char bBuffer[STRING_LENS_4096];
	int k = 0;
	
	DWORD dwStart = GetTickCount();

	if(!pMain->CryptStartup()) goto go_End;

	if(!(pInputFile=fopen(pMain->m_pszFile, _T("rb"))))
	{
		pMain->m_pLogger->OutputTracelog(_T("Fail to open the file, path: %s"), pMain->m_pszFile);
		pMain->CryptCleanup();
		goto go_End;
	}
	
	memset(&md5Hash, 0, sizeof(MD5Context));
	pMain->MD5Init(&md5Hash);
	while(!feof(pInputFile))
	{
		unsigned int nCount = fread(bBuffer, sizeof(unsigned char), STRING_LENS_4096, pInputFile);
		pMain->MD5Update(&md5Hash, bBuffer, nCount);
	}
	pMain->MD5Final(&md5Hash);
	fclose(pInputFile);

	for(int i = 0; i < 16; i++)
	{
		unsigned char b = md5Hash.digest[i];
		for(int j = 4; j >= 0; j -= 4)
		{
			char c = ((char)(b >> j) & 0x0F);
			if(c < 10) c += '0';
			else c = ('a' + (c - 10));
			pMain->m_md5Value[k] = c;
			k++;
		}
	}
	pMain->m_md5Value[k] = '\0';
	pMain->CryptCleanup();

go_End:
	printf("> Timecost: %2.3fsec\r\n", ((double)(GetTickCount()-dwStart))/1000.0);
	SetEvent(pMain->m_hThreadFinish);
	return 0;
}

bool myMD5::CryptStartup()
{
	bool bRtn = false;
	DWORD dwRtn;

	if(CryptAcquireContext(&m_hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == 0)
	{
		if((dwRtn=GetLastError()) == NTE_EXISTS)
		{
			if(CryptAcquireContext(&m_hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0) == 0)
			{
				m_pLogger->OutputTracelog(_T("call CryptAcquireContext failed, errCode: %d"), GetLastError());
				goto go_End;
			}
		}else
		{
			m_pLogger->OutputTracelog(_T("call CryptAcquireContext failed, errCode: %d"), dwRtn);
			goto go_End;
		}
	}

	bRtn = true;
go_End:
	return bRtn;
}

void myMD5::CryptCleanup()
{
	if(m_hCryptProv) CryptReleaseContext(m_hCryptProv, 0);
	m_hCryptProv = 0;
}

void myMD5::MD5Init(MD5Context *ctx)
{
	CryptCreateHash(m_hCryptProv, CALG_MD5, 0, 0, &ctx->hHash);
}

void myMD5::MD5Update(MD5Context *ctx, unsigned char const *pBuf, unsigned int Lens)
{
	CryptHashData(ctx->hHash, pBuf, Lens, 0);
}

void myMD5::MD5Final(MD5Context *ctx)
{
	DWORD dwCount = 16;
	CryptGetHashParam(ctx->hHash, HP_HASHVAL, ctx->digest, &dwCount, 0);
	if(ctx->hHash) CryptDestroyHash(ctx->hHash);
	ctx->hHash = 0;
}