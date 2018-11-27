#include "StdAfx.h"
#include "NI_GIT.h"


CNI_GIT::CNI_GIT(void)
{
}

CNI_GIT::~CNI_GIT(void)
{
}

void CNI_GIT::InitializeEquip(CLogger *pLogger)
{
	m_pLogger = pLogger;
	m_Socket.InitializeSocket(m_pLogger);
}

void CNI_GIT::TerminateEquip()
{
	m_Socket.CloseSocket();
}

bool CNI_GIT::ConnectDevice(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	NIDEV_SETTINGS *niSettings = NULL;

	if(dataStruct) niSettings = (NIDEV_SETTINGS*)dataStruct;
	else
	{
		strcpy_s(szRespMsg, nLens, _T("dataStruct is null"));
		goto go_Error;
	}

	if(!(bRtn=m_Socket.OpenSocket(niSettings->IPAddr, niSettings->Port)))
	{
		strcpy_s(szRespMsg, nLens, _T("open socket failed"));
		goto go_Error;
	}

	memset(&m_RecvBuffer, 0, STRING_LENS_4096);
	if(!(bRtn=m_Socket.SendSocketCmd(_T("READY?::"), RESPONSE_TIMEOUT, m_RecvBuffer, STRING_LENS_4096)))
	{
		strcpy_s(szRespMsg, nLens, _T("call SendSocketCmd fail"));
		goto go_Error;
	}

	return (m_Initialized=(strcmp(m_RecvBuffer, _T("READY::")) == 0));
go_Error:
	return bRtn;
}

bool CNI_GIT::SetupVSG(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;

	return bRtn;
}

bool CNI_GIT::SetupVSA(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;

	return bRtn;
}

bool CNI_GIT::StartMeasure(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;

	return bRtn;
}

bool CNI_GIT::StartGenerate(int frameCnt, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;

	return bRtn;
}

bool CNI_GIT::DisconnectDevice(LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;

	return bRtn;
}
