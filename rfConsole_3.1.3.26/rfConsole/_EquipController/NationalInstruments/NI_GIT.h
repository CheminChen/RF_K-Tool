#pragma once

#include "..\EquipCtrlBase.h"
#include "..\..\_Utility\mySocket\mySocket.h"

#define RESPONSE_TIMEOUT		3

class CNI_GIT : public EquipCtrlbase
{
public:
	CNI_GIT(void);
	~CNI_GIT(void);

public:
	void InitializeEquip(CLogger *pLogger);
	void TerminateEquip();

	bool ConnectDevice(void *dataStruct, LPSTR szRespMsg, int nLens);

	bool SetupVSG(void *dataStruct, LPSTR szRespMsg, int nLens);
	bool SetupVSA(void *dataStruct, LPSTR szRespMsg, int nLens);

	bool StartMeasure(void *dataStruct, LPSTR szRespMsg, int nLens);
	bool StartGenerate(int frameCnt, LPSTR szRespMsg, int nLens);

	bool DisconnectDevice(LPSTR szRespMsg, int nLens);

	bool					m_Initialized;

private:

	CLogger					*m_pLogger;
	mySocket				m_Socket;

	TCHAR					m_RecvBuffer[STRING_LENS_4096];

	IQANALYZE_SETTING		m_IQAnalySetting;
	IQVSA_SETTINGS			m_IQVSASettings;
	IQVSG_SETTINGS			m_IQVSGSettings;
};

