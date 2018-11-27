#include "StdAfx.h"
#include "TestItems.h"

RF_TYPE CTestItems::m_rfMeasurer;
TCHAR CTestItems::m_szRespMsg[STRING_LENS_4096];

mySocket CTestItems::m_Socket;
EquipCtrlbase *CTestItems::m_EquipCtrl = NULL;
CLogger *CTestItems::m_pLogger = NULL;

CTestItems::CTestItems(void)
{
	m_hSpectrumMask = NULL;
	m_pDutSettings = NULL;
	m_pTestResult = NULL;
	m_pLogger = NULL;
	m_EquipCtrl = NULL;
	m_rfMeasurer = RF_IQXEL_80;
	memset(m_szRespMsg, 0, STRING_LENS_4096);

	m_csv2gTxTitle.Add(_T("Timestamp")); m_csv5gTxTitle.Add(_T("Timestamp")); m_csv2gRxTitle.Add(_T("Timestamp")); m_csv5gRxTitle.Add(_T("Timestamp")); 
	m_csv2gTxTitle.Add(_T("MAC")); m_csv5gTxTitle.Add(_T("MAC")); m_csv2gRxTitle.Add(_T("MAC")); m_csv5gRxTitle.Add(_T("MAC")); 
	m_csv2gTxTitle.Add(_T("SN")); m_csv5gTxTitle.Add(_T("SN")); m_csv2gRxTitle.Add(_T("SN")); m_csv5gRxTitle.Add(_T("SN")); 
}

CTestItems::~CTestItems(void)
{
}

void CTestItems::InitializeTestItem(DUT_SETTINGS *pDutSettings, TEST_RESULT *pTestResult, CParser *pParser, CLogger *pLogger, bool bCsvExport)
{
	m_EquipCtrl = NULL;

	m_pParser = pParser;
	m_pLogger = pLogger;
	m_pDutSettings = pDutSettings;
	m_pTestResult = pTestResult;

	m_CsvExport = bCsvExport;
	m_Socket.InitializeSocket(m_pLogger);
}

void CTestItems::TerminateTestItem()
{
	/*vDUT*/
	TerminateDUT();

	m_Socket.CloseTelnet();
	
	if(m_EquipCtrl)
	{
		m_EquipCtrl->TerminateEquip();
		delete m_EquipCtrl;
		m_EquipCtrl = NULL;
	}
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

bool CTestItems::openTelnet(TCHAR* szIPAddr, TCHAR* szLoginPrompt, TCHAR* szLoginID, TCHAR* szPwdPrompt, TCHAR* szLoginPwd, TCHAR *szShellPrompt, LPSTR szRespMsg, int nLens)
{
	return m_Socket.OpenTelnet(szIPAddr, szLoginPrompt, szLoginID, szPwdPrompt, szLoginPwd, szShellPrompt, szRespMsg, nLens);
}

void CTestItems::closeTelnet()
{
	m_Socket.CloseTelnet();
}

bool CTestItems::sendTelnetCmd(TCHAR* szCmd, TCHAR* szChkString, int nTimeout, LPSTR szRespMsg, int nLens, bool bChkResponse)
{
	return m_Socket.SendTelnetCmd(szCmd, szChkString, nTimeout, szRespMsg, nLens, bChkResponse);
}

bool CTestItems::getTesterLock(LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;

	if(m_EquipCtrl)
	{
		if(((m_rfMeasurer == RF_IQXEL_80) || (m_rfMeasurer == RF_IQXEL_M8) || (m_rfMeasurer == RF_IQXEL_M2W)) && (((CLPIQ*)m_EquipCtrl)->m_Initialized))
		{
			bRtn = ((CLPIQ*)m_EquipCtrl)->GetLock(szRespMsg, nLens);
		}
	}

	return bRtn;
}

bool CTestItems::releaseTesterLock(LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	if(m_EquipCtrl)
	{
		if(((m_rfMeasurer == RF_IQXEL_80) || (m_rfMeasurer == RF_IQXEL_M8) || (m_rfMeasurer == RF_IQXEL_M2W)) && (((CLPIQ*)m_EquipCtrl)->m_Initialized))
		{
			bRtn = ((CLPIQ*)m_EquipCtrl)->ReleaseLock(szRespMsg, nLens);
		}
	}

	return bRtn;
}

bool CTestItems::setupVSA(RF_SETTINGS *pRFSettings, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	void *pVSASetting = NULL;

	if(m_EquipCtrl)
	{
		if(((m_rfMeasurer == RF_IQXEL_80) || (m_rfMeasurer == RF_IQXEL_M8) || (m_rfMeasurer == RF_IQXEL_M2W)) && (((CLPIQ*)m_EquipCtrl)->m_Initialized))
		{
			pVSASetting = new IQVSA_SETTINGS;
			memset(pVSASetting, 0, sizeof(IQVSA_SETTINGS));

			((IQVSA_SETTINGS*)pVSASetting)->Pathloss = pRFSettings->Pathloss;
			strcpy_s(((IQVSA_SETTINGS*)pVSASetting)->szRefFile, sizeof(((IQVSA_SETTINGS*)pVSASetting)->szRefFile), pRFSettings->szRefFile);
			for(int n=0 ; n<4 ; n++) ((IQVSA_SETTINGS*)pVSASetting)->AllPathloss[n] = pRFSettings->AllPathloss[n];

			((IQVSA_SETTINGS*)pVSASetting)->Frequency = (double)CConverter::Channel2Frequency(pRFSettings->Channel);
			((IQVSA_SETTINGS*)pVSASetting)->rfAmplDb = pRFSettings->TxPower + CConverter::PeakToAvgPower(pRFSettings->Modulation) - ((IQVSA_SETTINGS*)pVSASetting)->Pathloss;
			((IQVSA_SETTINGS*)pVSASetting)->triggerLevelDb = -25;	/*???, Needs to confirm*/
			((IQVSA_SETTINGS*)pVSASetting)->triggerPreTime = 5e-6;	/*???, Needs to confirm*/
			((IQVSA_SETTINGS*)pVSASetting)->samplingTimeSecs = CConverter::Modulation2SamplingTime(pRFSettings->Modulation);
			((IQVSA_SETTINGS*)pVSASetting)->triggerType = (pRFSettings->Modulation == MODULATION_CW)?IQV_TRIG_TYPE_FREE_RUN:IQV_TRIG_TYPE_IF2_NO_CAL;
			((IQVSA_SETTINGS*)pVSASetting)->ht40Mode = CConverter::Bandwidth2CaptureType(pRFSettings->BandWidth);   
			((IQVSA_SETTINGS*)pVSASetting)->AnalyzeType = pRFSettings->AnalyzeType;
			((IQVSA_SETTINGS*)pVSASetting)->BandWidth = pRFSettings->BandWidth;
			
			memset(((IQVSA_SETTINGS*)pVSASetting)->wifi_mode, 0, sizeof(((IQVSA_SETTINGS*)pVSASetting)->wifi_mode));
			if((pRFSettings->AntSel == TX_CHAIN_AB) || (pRFSettings->AntSel == TX_CHAIN_ABC) || (pRFSettings->AntSel == TX_CHAIN_ALL)) strcpy_s(((IQVSA_SETTINGS*)pVSASetting)->wifi_mode, sizeof(((IQVSA_SETTINGS*)pVSASetting)->wifi_mode), _T("composite"));
			else strcpy_s(((IQVSA_SETTINGS*)pVSASetting)->wifi_mode, sizeof(((IQVSA_SETTINGS*)pVSASetting)->wifi_mode), _T("nxn"));

			if(!(m_EquipCtrl->SetupVSA(pVSASetting, szRespMsg, nLens))) goto go_End;
			else bRtn = true;

		}else if(m_rfMeasurer == RF_NI)
		{

		}
	}

go_End:
	DEL_ITEM(pVSASetting);
	return bRtn;
}

bool CTestItems::startMeasure(MEASURE_RESULT *pMeasureResult, LPSTR szRespMsg, int nLens)
{	
	return (m_EquipCtrl && (m_EquipCtrl->StartMeasure(pMeasureResult, szRespMsg, nLens)));
}

bool CTestItems::startGenerate(int frameCnt, LPSTR szRespMsg, int nLens)
{
	return (m_EquipCtrl && (m_EquipCtrl->StartGenerate(frameCnt, szRespMsg, nLens)));
}

void CTestItems::outputLog(LOGTYPE logType, TCHAR *szFormat, ...)
{
	va_list marker;
	TCHAR *pszBuffer = new TCHAR[MAX_ARCBUFFER_SIZE];
	memset(pszBuffer, _T('\0'), MAX_ARCBUFFER_SIZE);

	va_start(marker, szFormat);
	vsprintf_s(pszBuffer, MAX_ARCBUFFER_SIZE, szFormat, marker);
	va_end(marker);

	if(m_pLogger) m_pLogger->OutputLog(logType, COLOR_WHITE, pszBuffer);

	DEL_ARRAY(pszBuffer)
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

RUNSTATUS CTestItems::ARC_FORTEST(CString strParameter)
{
	m_Ping.PingUntilAilve(_T("192.168.72.1"), 100);

	/*RF_SETTINGS *pRFSettings = NULL;
	SPEC_SETTINGS *pSPECSettings = NULL;
	MEASURE_RESULT *pMeasureResult = NULL;
	WIFI_TXRESULT *pWIFITx = NULL;
	RF_MODE rfMode = RF_TX;	
	bool bFlag;

	TCHAR szRefFile[MAX_PATH] = {'\0'};
	
	pRFSettings = new RF_SETTINGS;
	memset(pRFSettings, 0, sizeof(RF_SETTINGS));	
	m_pParser->GetTestParameter(strParameter, _T("REF_FILE"), szRefFile, MAX_PATH);
	//if(strcmp(szRefFile, _T("")) != 0) sprintf_s(pRFSettings->szRefFile, MAX_PATH, _T("%s\\Mod\\%s"), szRefFile);
	if(strcmp(szRefFile, _T("")) != 0) sprintf_s(pRFSettings->szRefFile, MAX_PATH, _T("%s\\Mod\\%s"), CCommFunc::m_ModulePath, szRefFile);

	if(!(
		(bFlag = m_pParser->GetTestParameter(strParameter, _T("ANALYZE"), &pRFSettings->AnalyzeType)) &&
		(bFlag = m_pParser->GetTestParameter(strParameter, _T("CH"), &pRFSettings->Channel) || m_pParser->GetTestParameter(strParameter, _T("FREQ"), &pRFSettings->Frequency)) &&
		(bFlag = m_pParser->GetTestParameter(strParameter, _T("RATE"), &pRFSettings->DataRate)) && 
		(bFlag = m_pParser->GetTestParameter(strParameter, _T("BW"), &pRFSettings->BandWidth)) &&
		(bFlag = m_pParser->GetTestParameter(strParameter, _T("ANT"), &pRFSettings->AntSel)) && 
		(bFlag = m_pParser->GetTestParameter(strParameter, _T("POWER"), &pRFSettings->TxPower)) &&
		(bFlag = m_pParser->GetPathlossByCH(pRFSettings->Channel, pRFSettings->AntSel, &pRFSettings->Pathloss, pRFSettings->AllPathloss))
		))	
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Test parameter incorrect"));
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_VERIFY_ALL"), _T("Test parameter incorrect"));

		goto go_End;
	}
	pRFSettings->Modulation = CConverter::DataRate2Modulation(pRFSettings->DataRate);

	if(!getTesterLock(m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("getTesterLock failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!setupVSA(pRFSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("setupVSA failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	pMeasureResult = new MEASURE_RESULT();
	pWIFITx = new WIFI_TXRESULT;
	pMeasureResult->AnalyzeType = pRFSettings->AnalyzeType;
	pMeasureResult->TestResult = pWIFITx;

	int nRetry = 3;

	do 
	{
		Sleep(500);

		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!m_EquipCtrl->StartMeasure(pMeasureResult, m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("RespMsg: %s"), m_szRespMsg);
			continue;;		
		}

	} while (--nRetry > 0);
	

	WIFI_TXRESULT *pResult = (WIFI_TXRESULT *)pMeasureResult->TestResult;
	for(int n=0 ; n<4 ; n++)
	{
		printf("power: %f\n", pResult->CompositePower[n]);
	}

	if(!releaseTesterLock(m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("releaseTesterLock failed, errMsg: %s"), m_szRespMsg);
		goto go_End;
	}

go_End:*/
	return STATUS_PASS;
}

RUNSTATUS CTestItems::ARC_PING_TO_ALIVE(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	int nPingTimeout = 0, nWaitReady = 0;
	TCHAR IPAddr[IP_ADDRESS] = {'\0'};
	
	m_pParser->GetTestParameter(strParameter, _T("WAIT_READY"), &nWaitReady);
	if(!((m_pParser->GetTestParameter(strParameter, _T("TIMEOUT"), &nPingTimeout)) &&
		(m_pParser->GetTestParameter(strParameter, _T("IP"), IPAddr, IP_ADDRESS)))
	  )

	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Input parameter incorrect, Please check."));
		SetErrorReport(_T(""), _T("ARC_PING_TO_ALIVE"), _T("Input parameter incorrect, Param: '%s'"), strParameter);

		goto go_End;
	}

	rsRtn = m_Ping.PingUntilAilve(IPAddr, nPingTimeout)?STATUS_PASS:STATUS_FUNCTION_ERROR;

go_End:
	return rsRtn;
}

RUNSTATUS CTestItems::ARC_PING_TO_DEAD(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	int nPingTimeout = 0;
	TCHAR IPAddr[IP_ADDRESS] = {'\0'};

	if(!((m_pParser->GetTestParameter(strParameter, _T("TIMEOUT"), &nPingTimeout)) &&
		(m_pParser->GetTestParameter(strParameter, _T("IP"), IPAddr, IP_ADDRESS)))
		)

	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Input parameter incorrect, Please check."));
		SetErrorReport(_T(""), _T("ARC_PING_TO_ALIVE"), _T("Input parameter incorrect, Param: '%s'"), strParameter);

		goto go_End;
	}

	rsRtn = m_Ping.PingUntilDead(IPAddr, nPingTimeout)?STATUS_PASS:STATUS_FUNCTION_ERROR;

go_End:
	return rsRtn;
}

RUNSTATUS CTestItems::ATC_CONNECT_TESTER(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;

	m_pParser->GetTestParameter(strParameter, _T("MEASURER"), &m_rfMeasurer);

	if((m_rfMeasurer == RF_IQXEL_80) || (m_rfMeasurer == RF_IQXEL_M8) || (m_rfMeasurer == RF_IQXEL_M2W))
	{
		if((m_EquipCtrl != NULL) && ((CLPIQ*)m_EquipCtrl)->m_Initialized)
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("IQ device has already initialized."));
			rsRtn = STATUS_PASS;
			goto go_End;
		}

		rsRtn = Init_IQDevice(strParameter)?STATUS_PASS:STATUS_DEVICE_ERROR;

	}else if(m_rfMeasurer == RF_NI)
	{
		if((m_EquipCtrl != NULL) && ((CNI_GIT*)m_EquipCtrl)->m_Initialized)
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("NI-GIT server has already initialized."));
			rsRtn = STATUS_PASS;
			goto go_End;
		}

		rsRtn = Init_NIDevice(strParameter)?STATUS_PASS:STATUS_DEVICE_ERROR;
	}

go_End:
	return rsRtn;
}

RUNSTATUS CTestItems::ATC_TESTER_RELEASE_CONTROL()
{
	RUNSTATUS rsRtn = STATUS_DEVICE_ERROR;
	memset(m_szRespMsg, 0, STRING_LENS_4096);

	if((m_rfMeasurer == RF_IQXEL_80) || (m_rfMeasurer == RF_IQXEL_M8) || (m_rfMeasurer == RF_IQXEL_M2W))
	{
		if((m_EquipCtrl != NULL) && ((CLPIQ*)m_EquipCtrl)->m_Initialized)
		{
			if(!((CLPIQ*)m_EquipCtrl)->ReleaseCtrlDevice(m_szRespMsg, STRING_LENS_4096))
			{
				m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("ReleaseCtrlDevice failed, errMsg: %s"), m_szRespMsg);
				SetErrorReport(_T(""), _T("ATC_TESTER_RELEASE_CONTROL"), _T("ReleaseCtrlDevice failed, errMsg: %s"), m_szRespMsg);

				goto go_End;
			}else rsRtn = STATUS_PASS;

			//((CLPIQ*)m_EquipCtrl)->m_Initialized = false;
		}else rsRtn = STATUS_PASS;
	}

	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("IQ device has already release control"));
go_End:
	return rsRtn;
}

RUNSTATUS CTestItems::LOAD_IQ_PARAMETER(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	IQANALYZE_SETTING *pIQAnalySetting = new IQANALYZE_SETTING();

	memset(m_szRespMsg, 0, STRING_LENS_4096);

	/*Customize Settings*/
	//11B
	m_pParser->GetTestParameter(strParameter, _T("11b_eq_taps"), &pIQAnalySetting->wifi_11b_eq_taps, 1);
	m_pParser->GetTestParameter(strParameter, _T("11b_DCremove11b_flag"), &pIQAnalySetting->wifi_11b_DCremove11b_flag, 0);
	m_pParser->GetTestParameter(strParameter, _T("11b_method_11b"), &pIQAnalySetting->wifi_11b_method_11b, 1);

	//11AG
	m_pParser->GetTestParameter(strParameter, _T("11ag_ph_corr_mode"), &pIQAnalySetting->wifi_11ag_ph_corr_mode, 2);
	m_pParser->GetTestParameter(strParameter, _T("11ag_ch_estimate"), &pIQAnalySetting->wifi_11ag_ch_estimate, 1);
	m_pParser->GetTestParameter(strParameter, _T("11ag_sym_tim_corr"), &pIQAnalySetting->wifi_11ag_sym_tim_corr, 2);
	m_pParser->GetTestParameter(strParameter, _T("11ag_freq_sync"), &pIQAnalySetting->wifi_11ag_freq_sync, 2);
	m_pParser->GetTestParameter(strParameter, _T("11ag_ampl_track"), &pIQAnalySetting->wifi_11ag_ampl_track, 2);	//???s

	//11N
	m_pParser->GetTestParameter(strParameter, _T("11n_enablePhaseCorr"), &pIQAnalySetting->wifi_11n_enablePhaseCorr, 1);
	m_pParser->GetTestParameter(strParameter, _T("11n_enableSymTimingCorr"), &pIQAnalySetting->wifi_11n_enableSymTimingCorr, 1);
	m_pParser->GetTestParameter(strParameter, _T("11n_enableAmplitudeTracking"), &pIQAnalySetting->wifi_11n_enableAmplitudeTracking, 0);
	m_pParser->GetTestParameter(strParameter, _T("11n_decodePSDU"), &pIQAnalySetting->wifi_11n_decodePSDU, 1);
	m_pParser->GetTestParameter(strParameter, _T("11n_enableFullPacketChannelEst"), &pIQAnalySetting->wifi_11n_enableFullPacketChannelEst, 0);
	m_pParser->GetTestParameter(strParameter, _T("11n_packetFormat"), &pIQAnalySetting->wifi_11n_packetFormat, 0);
	m_pParser->GetTestParameter(strParameter, _T("11n_frequencyCorr"), &pIQAnalySetting->wifi_11n_frequencyCorr, 2);

	//AC
	m_pParser->GetTestParameter(strParameter, _T("11ac_enablePhaseCorr"), &pIQAnalySetting->wifi_11ac_enablePhaseCorr, 1);
	m_pParser->GetTestParameter(strParameter, _T("11ac_enableSymTimingCorr"), &pIQAnalySetting->wifi_11ac_enableSymTimingCorr, 1);
	m_pParser->GetTestParameter(strParameter, _T("11ac_enableAmplitudeTracking"), &pIQAnalySetting->wifi_11ac_enableAmplitudeTracking, 0);
	m_pParser->GetTestParameter(strParameter, _T("11ac_decodePSDU"), &pIQAnalySetting->wifi_11ac_decodePSDU, 0);
	m_pParser->GetTestParameter(strParameter, _T("11ac_enableFullPacketChannelEst"), &pIQAnalySetting->wifi_11ac_enableFullPacketChannelEst, 0);
	m_pParser->GetTestParameter(strParameter, _T("11ac_frequencyCorr"), &pIQAnalySetting->wifi_11ac_frequencyCorr, 2);
	m_pParser->GetTestParameter(strParameter, _T("11ac_packetFormat"), &pIQAnalySetting->wifi_11ac_packetFormat, 4);

	if((m_EquipCtrl != NULL) && ((CLPIQ*)m_EquipCtrl)->m_Initialized)
	{
		((CLPIQ*)m_EquipCtrl)->SetAnalyzeSetting(pIQAnalySetting);	
	}	

	ShowIQParameter(pIQAnalySetting);
	if(pIQAnalySetting) delete pIQAnalySetting;
	return STATUS_PASS;
}

RUNSTATUS CTestItems::SETUP_SPECTRUM_MASK(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	TCHAR titleText[MAX_PATH] = {0};
	HWND nHwnd = ::GetWindow(::GetDesktopWindow(), GW_CHILD);

	if(m_hSpectrumMask == NULL)
	{
		while (nHwnd != NULL)
		{
			::GetWindowText(nHwnd, titleText, MAX_PATH);
			if((strlen(titleText)) && (strcmp(titleText, GUI_WIN_NAME) == 0))
			{
				m_hSpectrumMask = nHwnd;
				break;
			}

			nHwnd = ::GetWindow(nHwnd, GW_HWNDNEXT);
		}
	}	

	if(m_hSpectrumMask)
	{
		if((m_EquipCtrl != NULL) && ((CLPIQ*)m_EquipCtrl)->m_Initialized)
		{
			((CLPIQ*)m_EquipCtrl)->SetupSpectrumMask(&m_hSpectrumMask);
			rsRtn = STATUS_PASS;			
		}
	}

	if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Find the '%s' tool %s"), GUI_WIN_NAME, (rsRtn == STATUS_PASS)?_T("success"):_T("fail"));
	return rsRtn;
}

RUNSTATUS CTestItems::LOAD_PATHLOSS(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	TCHAR *pFile = new TCHAR[MAX_PATH], *pTmp = new TCHAR[MAX_PATH];
	memset(m_szRespMsg, 0, STRING_LENS_4096);

	if(!m_pParser->GetTestParameter(strParameter, _T("PATH"), pTmp, MAX_PATH))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Input parameter incorrect, Please check."));
		SetErrorReport(_T(""), _T("LOAD_PATHLOSS"), _T("Input parameter incorrect, Param: '%s'"), strParameter);

		goto go_End;
	}
	sprintf_s(pFile, MAX_PATH, _T("%s\\%s"), CCommFunc::m_ModulePath, pTmp);

	if(!(CCommFunc::FileExist(pFile) && m_pParser->ParserPathloss(pFile, m_pLogger)))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Incorrect file format or file doesn't exist, path: %s"), pFile);
		SetErrorReport(_T(""), _T("LOAD_PATHLOSS"), _T("Incorrect file format or file doesn't exist, path: %s"), pFile);

		goto go_End;
	}

	rsRtn = STATUS_PASS;
go_End:
	DEL_ARRAY(pFile)
	DEL_ARRAY(pTmp)
	return rsRtn;
}

RUNSTATUS CTestItems::ATC_REBOOT_DUT(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	int nWaitReady = 0;

	memset(m_szRespMsg, 0, STRING_LENS_4096);

	m_pParser->GetTestParameter(strParameter, _T("WAITREADY"), &nWaitReady);
	if(!sendTelnetCmd(_T("reboot"), _T("#"), 3, m_szRespMsg, STRING_LENS_4096, !m_pDutSettings->IgnoreResponse))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, m_szRespMsg);
		SetErrorReport(_T(""), _T("ATC_REBOOT_DUT"), m_szRespMsg);

		goto go_End;
	}
	if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("DUT has be reboot successfully and wait %ds"), nWaitReady);
	
	rsRtn = STATUS_PASS;
go_End:
	return rsRtn;
}

RUNSTATUS CTestItems::ATC_INSERT_DUT(CString strParameter)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	int nWaitReady = 0;
	
	memset(m_szRespMsg, 0, STRING_LENS_4096);

	m_pParser->GetTestParameter(strParameter, _T("WAITREADY"), &nWaitReady);

	if(m_pDutSettings->testType == TTYPE_TELNET)
	{
		if(strParameter.Find(_T("IF2G=")) != -1) m_pParser->GetTestParameter(strParameter, _T("IF2G"), m_pDutSettings->wifiIF, STRING_LENS_16);
		else if(strParameter.Find(_T("IF5G=")) != -1) m_pParser->GetTestParameter(strParameter, _T("IF5G"), m_pDutSettings->wifiIF, STRING_LENS_16);

		InitializeDUT(CTestItems::setupVSA, CTestItems::setupVSG, CTestItems::startMeasure, CTestItems::startGenerate, CTestItems::getTesterLock, CTestItems::releaseTesterLock, CTestItems::outputLog, CTestItems::openTelnet, CTestItems::closeTelnet, CTestItems::sendTelnetCmd);
	}
	else InitializeDUT(CTestItems::setupVSA, CTestItems::setupVSG, CTestItems::startMeasure, CTestItems::startGenerate, CTestItems::getTesterLock, CTestItems::releaseTesterLock, CTestItems::outputLog, NULL, NULL, NULL);
		
	/*vDUT_Init*/
	if(!(vDUT_Init(m_pDutSettings, m_szRespMsg, STRING_LENS_4096)))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, m_szRespMsg);
		SetErrorReport(_T(""), _T("ATC_INSERT_DUT"), m_szRespMsg);

		goto go_End;
	}
	if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("DUT has be initialized successfully"));

	if(nWaitReady != 0)
	{
		if(!vDUT_WaitReady(nWaitReady, m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, m_szRespMsg);
			SetErrorReport(_T(""), _T("ATC_INSERT_DUT"), m_szRespMsg);

			goto go_End;
		}
	}
	
	rsRtn = STATUS_PASS;
go_End:
	return rsRtn;
}

RUNSTATUS CTestItems::ATC_REMOVE_DUT()
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;

	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!vDUT_Remove(m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, m_szRespMsg);
		SetErrorReport(_T(""), _T("ATC_REMOVE_DUT"), m_szRespMsg);

		goto go_End;
	}
	if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("DUT has be removed successfully"));

	rsRtn = STATUS_PASS;
go_End:
	return rsRtn;
}

RUNSTATUS CTestItems::WIFI_11AC_RX_CALIBRATION(CString strParameter, CString strSPEC, CString strOptions)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	RF_SETTINGS *pRFSettings = NULL;
	int frameCnt = 0;

	memset(m_szRespMsg, 0, STRING_LENS_4096);

	if(!checkEquipCtrl(_T("WIFI_11AC_TX_VERIFY_ALL"))) goto go_End;	

	/*vDUT_Init*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!(vDUT_Init(m_pDutSettings, m_szRespMsg, STRING_LENS_4096)))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("WIFI_11AC_RX_CALIBRATION"), _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	pRFSettings = new RF_SETTINGS;
	memset(pRFSettings, 0, sizeof(RF_SETTINGS));
	m_pParser->GetTestParameter(strSPEC, _T("GAIN_LIMIT"), &pRFSettings->RxGainLimit);
	if(!(
		(m_pParser->GetTestParameter(strParameter, _T("ANALYZE"), &pRFSettings->AnalyzeType)) && (m_pParser->GetTestParameter(strParameter, _T("CH"), &pRFSettings->Channel)) &&
		(m_pParser->GetTestParameter(strParameter, _T("RATE"), &pRFSettings->DataRate)) && (m_pParser->GetTestParameter(strParameter, _T("BW"), &pRFSettings->BandWidth)) &&
		(m_pParser->GetTestParameter(strParameter, _T("ANT"), &pRFSettings->AntSel)) && (m_pParser->GetTestParameter(strParameter, _T("POWER"), &pRFSettings->RxPower)) &&
		(m_pParser->GetTestParameter(strParameter, _T("PACKET"), &frameCnt)) && (m_pParser->GetTestParameter(strParameter, _T("WAVEFORM"), pRFSettings->szModFile, MAX_PATH)) &&
		(m_pParser->GetPathlossByCH(pRFSettings->Channel, pRFSettings->AntSel, &pRFSettings->Pathloss, pRFSettings->AllPathloss))
		))	
	{		
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Test parameter incorrect"));
		SetErrorReport(_T(""), _T("WIFI_11AC_RX_VERIFY_PER"), _T("Test parameter incorrect"));

		goto go_End;
	}

	if(!vDUT_Calibration(RX_CAL, pRFSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Calibration failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_CALIBRATION"), _T("vDUT_Calibration failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	rsRtn = STATUS_PASS;
go_End:
	DEL_ITEM(pRFSettings)
		return rsRtn;
}

RUNSTATUS CTestItems::WIFI_11AC_TX_CALIBRATION(CString strParameter, CString strSPEC, CString strOptions)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	RF_SETTINGS *pRFSettings = NULL;
	TCHAR *pszRefFile = new TCHAR[MAX_PATH];

	memset(m_szRespMsg, 0, STRING_LENS_4096);
	memset(pszRefFile, 0, MAX_PATH);

	if(!checkEquipCtrl(_T("WIFI_11AC_TX_CALIBRATION"))) goto go_End;	

	/*vDUT_Init*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!(vDUT_Init(m_pDutSettings, m_szRespMsg, STRING_LENS_4096)))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_CALIBRATION"), _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}
	
	pRFSettings = new RF_SETTINGS;
	memset(pRFSettings, 0, sizeof(RF_SETTINGS));	
	m_pParser->GetTestParameter(strParameter, _T("ANTNUM"), &pRFSettings->AntCalNum);
	m_pParser->GetTestParameter(strParameter, _T("BANDNUM"), &pRFSettings->BandCalNum);
	m_pParser->GetTestParameter(strParameter, _T("REF_FILE"), pszRefFile, MAX_PATH);
	if(strcmp(pszRefFile, _T("")) != 0) sprintf_s(pRFSettings->szRefFile, MAX_PATH, _T("%s\\Mod\\%s"), CCommFunc::m_ModulePath, pszRefFile);
	m_pParser->GetTestParameter(strSPEC, _T("PWR_LIMIT"), &pRFSettings->PwrCalLimit);
	if(!(
		(m_pParser->GetTestParameter(strParameter, _T("ANALYZE"), &pRFSettings->AnalyzeType)) &&
		(m_pParser->GetTestParameter(strParameter, _T("CH"), &pRFSettings->Channel) || m_pParser->GetTestParameter(strParameter, _T("FREQ"), &pRFSettings->Frequency)) &&
		(m_pParser->GetTestParameter(strParameter, _T("RATE"), &pRFSettings->DataRate)) && (m_pParser->GetTestParameter(strParameter, _T("BW"), &pRFSettings->BandWidth)) &&
		(m_pParser->GetTestParameter(strParameter, _T("ANT"), &pRFSettings->AntSel)) && (m_pParser->GetTestParameter(strParameter, _T("POWER"), &pRFSettings->TxPower)) &&
		(m_pParser->GetPathlossByCH(pRFSettings->Channel, pRFSettings->AntSel, &pRFSettings->Pathloss, pRFSettings->AllPathloss))
		))	
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Test parameter incorrect"));
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_CALIBRATION"), _T("Test parameter incorrect"));

		goto go_End;
	}
	pRFSettings->Modulation = CConverter::DataRate2Modulation(pRFSettings->DataRate);


	if(!vDUT_Calibration(TRIM_POWER, pRFSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Calibration failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_CALIBRATION"), _T("vDUT_Calibration failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	rsRtn = STATUS_PASS;
go_End:
	DEL_ITEM(pRFSettings)
	DEL_ARRAY(pszRefFile)
	return rsRtn;
}

RUNSTATUS CTestItems::WIFI_11AC_TX_VERIFY_ALL(CString strParameter, CString strSPEC, CString strOptions)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	double dbTimeCost = GetTickCount();
	int ReMeasure = 0, measureDelay = 0, analyFailCnt = 5;
	CString strTitle, strTestItem, strTmp;
	bool testResult = false, mesasureDebug = false, bIs2G;

	RF_SETTINGS *pRFSettings = NULL;
	SPEC_SETTINGS *pSPECSettings = NULL;
	MEASURE_RESULT *pMeasureResult = NULL;
	WIFI_TXRESULT *pWIFITx = NULL;
	RF_MODE rfMode = RF_TX;	
		

	/*Parser*/
	m_pParser->GetTestParameter(strOptions, _T("REMEASURE"), &ReMeasure);
	m_pParser->GetTestParameter(strOptions, _T("DEBUG"), &mesasureDebug);
	m_pParser->GetTestParameter(strOptions, _T("MDELAY"), &measureDelay);

	if(!mesasureDebug && !checkEquipCtrl(_T("WIFI_11AC_TX_VERIFY_ALL"))) goto go_End;	

/*vDUT_Init*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!(vDUT_Init(m_pDutSettings, m_szRespMsg, STRING_LENS_4096)))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_VERIFY_ALL"), _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	pRFSettings = new RF_SETTINGS;
	memset(pRFSettings, 0, sizeof(RF_SETTINGS));	
	if(!(
		(m_pParser->GetTestParameter(strParameter, _T("ANALYZE"), &pRFSettings->AnalyzeType)) &&
		(m_pParser->GetTestParameter(strParameter, _T("CH"), &pRFSettings->Channel) || m_pParser->GetTestParameter(strParameter, _T("FREQ"), &pRFSettings->Frequency)) &&
		(m_pParser->GetTestParameter(strParameter, _T("RATE"), &pRFSettings->DataRate)) && (m_pParser->GetTestParameter(strParameter, _T("BW"), &pRFSettings->BandWidth)) &&
		(m_pParser->GetTestParameter(strParameter, _T("ANT"), &pRFSettings->AntSel)) && (m_pParser->GetTestParameter(strParameter, _T("POWER"), &pRFSettings->TxPower)) &&
		(m_pParser->GetPathlossByCH(pRFSettings->Channel, pRFSettings->AntSel, &pRFSettings->Pathloss, pRFSettings->AllPathloss))
		))	
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Test parameter incorrect"));
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_VERIFY_ALL"), _T("Test parameter incorrect"));

		goto go_End;
	}
	pRFSettings->Modulation = CConverter::DataRate2Modulation(pRFSettings->DataRate);

	//Parser SPEC
	pSPECSettings = new SPEC_SETTINGS();
	m_pParser->GetTestParameter(strSPEC, _T("POWER_H"), &pSPECSettings->PowerH);
	if(pSPECSettings->PowerH != NA_NUMBER) pSPECSettings->PowerH += pRFSettings->TxPower;
	m_pParser->GetTestParameter(strSPEC, _T("POWER_L"), &pSPECSettings->PowerL);
	if(pSPECSettings->PowerL != NA_NUMBER) pSPECSettings->PowerL = pRFSettings->TxPower - pSPECSettings->PowerL;
	m_pParser->GetTestParameter(strSPEC, _T("EVM"), &pSPECSettings->EVM);
	m_pParser->GetTestParameter(strSPEC, _T("MASK"), &pSPECSettings->SpectrumViolation);
	m_pParser->GetTestParameter(strSPEC, _T("PPM"), &pSPECSettings->FreqError);

	//Print Title
	if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Frequency: %.f, Data Rate: %s, Bandwidth: %s, Antenna: %s, Tx Power: %2.1f\r\n"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel), pRFSettings->TxPower);
	strTestItem.Format(_T("TX_%.f_%s_%s_%s"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel));

/*vDUT/ Equipment setTx*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!vDUT_SetTx(pRFSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_SetTx failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_SetTx failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

/*Get Tester Lock, 2018/8/15 */
	if(!getTesterLock(m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("getTesterLock failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("getTesterLock failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!mesasureDebug && !setupVSA(pRFSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("setupVSA failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("setupVSA failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	pMeasureResult = new MEASURE_RESULT();
	pWIFITx = new WIFI_TXRESULT;
	pMeasureResult->AnalyzeType = pRFSettings->AnalyzeType;
	pMeasureResult->TestResult = pWIFITx;
	strcpy_s(pWIFITx->TestItem, sizeof(pWIFITx->TestItem), strTestItem.GetBuffer());

	do 
	{
/*vDUT_Start*/
		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!vDUT_Start(RF_TX, NULL, m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Start failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_Start failed, errMsg: %s"), m_szRespMsg);

			goto go_End;
		}

/*Measure*/
		do 
		{
			Sleep(measureDelay);

			if(!mesasureDebug)
			{
				memset(m_szRespMsg, 0, STRING_LENS_4096);
				if(!mesasureDebug && !m_EquipCtrl->StartMeasure(pMeasureResult, m_szRespMsg, STRING_LENS_4096))
				{
					m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("ReMeasure: %d, analyFailCnt: %d, RespMsg: %s"), ReMeasure, analyFailCnt, m_szRespMsg);

					if(--analyFailCnt <= 0)
					{
						SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("EquipCtrl StartMeasure failed, RespMsg: %s"), m_szRespMsg);
						goto go_Stop;
					}					
					continue;
				}

			}else
			{
				printf("\n    [DEBUG] > Press Enter to continue\n");
				_getch();
			}

/*Compare Result*/
			if(!mesasureDebug)
			{
				if((testResult=wifiCompareTxResult(strTestItem, (WIFI_TXRESULT*)pMeasureResult->TestResult, pSPECSettings, ReMeasure))) break;
				else --ReMeasure;
			}else break;	

		} while ((ReMeasure >= 0) && (analyFailCnt > 0));

go_Stop:
		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!vDUT_Stop(RF_TX, NULL, m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Stop failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_Stop failed, errMsg: %s"), m_szRespMsg);

			goto go_End;
		}

		/*Release Tester Lock, 2018/8/15 */
		if(!releaseTesterLock(m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("releaseTesterLock failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("releaseTesterLock failed, errMsg: %s"), m_szRespMsg);

			goto go_Stop;
		}
	} while (false);

	/*Output CSV*/
	if(!mesasureDebug && m_CsvExport)
	{
		bIs2G = (pRFSettings->Channel <= 14);
		//printf("bIs2G: %s, Channel: %d", bIs2G?"T":"F", pRFSettings->Channel);

		strTmp.Format(_T("%.fMHz_%s_%s_%s"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel));
		(bIs2G)?m_csv2gTxTitle.Add(strTmp):m_csv5gTxTitle.Add(strTmp);

		if(pWIFITx->rmsPower != NA_NUMBER)
		{
			strTmp.Format(_T("%3.2f"), pWIFITx->rmsPower);
			(bIs2G)?m_csv2gTxPower.Add(strTmp):m_csv5gTxPower.Add(strTmp);
		}else (bIs2G)?m_csv2gTxPower.Add(_T("N/A")):m_csv5gTxPower.Add(_T("N/A"));

		if(pWIFITx->evmAll != NA_NUMBER)
		{
			strTmp.Format(_T("%3.2f"), pWIFITx->evmAll);
			(bIs2G)?m_csv2gEVM.Add(strTmp):m_csv5gEVM.Add(strTmp);
		}else (bIs2G)?m_csv2gEVM.Add(_T("N/A")):m_csv5gEVM.Add(_T("N/A"));
	}

	rsRtn = testResult?STATUS_PASS:STATUS_SPEC_FAIL;
	//if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Test time: %.2fsec"), (float)((GetTickCount() - dbTimeCost)/1000));
go_End:
	DEL_ITEM(pRFSettings);
	DEL_ITEM(pSPECSettings);
	DEL_ITEM(pWIFITx);
	DEL_ITEM(pMeasureResult);
	return rsRtn;
}

RUNSTATUS CTestItems::WIFI_11AC_RX_VERIFY_PER(CString strParameter, CString strSPEC, CString strOptions)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	RF_SETTINGS *pRFSettings = new RF_SETTINGS;
	SPEC_SETTINGS *pSPECSettings = new SPEC_SETTINGS();
	WIFI_RXRESULT *pWIFIRx = new WIFI_RXRESULT();
	TCHAR *pszModFile = new TCHAR[MAX_PATH];

	int ReMeasure = 0, frameCnt = 0;
	//double Pathloss;
	bool testResult = false, bIs2G;
	CString strTestItem = _T(""), strTmp;

	memset(pszModFile, 0, MAX_PATH);
	
	/*Parser*/
	m_pParser->GetTestParameter(strOptions, _T("REMEASURE"), &ReMeasure);

	if(!checkEquipCtrl(_T("WIFI_11AC_RX_VERIFY_PER"))) goto go_End;	

/*vDUT_Init*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!(vDUT_Init(m_pDutSettings, m_szRespMsg, STRING_LENS_4096)))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_VERIFY_ALL"), _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	//Parameter
	memset(pRFSettings, 0, sizeof(RF_SETTINGS));
	if(!(
		(m_pParser->GetTestParameter(strParameter, _T("ANALYZE"), &pRFSettings->AnalyzeType)) && (m_pParser->GetTestParameter(strParameter, _T("CH"), &pRFSettings->Channel)) &&
		(m_pParser->GetTestParameter(strParameter, _T("RATE"), &pRFSettings->DataRate)) && (m_pParser->GetTestParameter(strParameter, _T("BW"), &pRFSettings->BandWidth)) &&
		(m_pParser->GetTestParameter(strParameter, _T("ANT"), &pRFSettings->AntSel)) && (m_pParser->GetTestParameter(strParameter, _T("POWER"), &pRFSettings->RxPower)) &&
		(m_pParser->GetTestParameter(strParameter, _T("PACKET"), &frameCnt)) && (m_pParser->GetTestParameter(strParameter, _T("WAVEFORM"), pszModFile, MAX_PATH)) &&
		(m_pParser->GetPathlossByCH(pRFSettings->Channel, pRFSettings->AntSel, &pRFSettings->Pathloss, pRFSettings->AllPathloss))
		))	
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Test parameter incorrect"));
		SetErrorReport(_T(""), _T("WIFI_11AC_RX_VERIFY_PER"), _T("Test parameter incorrect"));

		goto go_End;
	}

	//SPEC
	m_pParser->GetTestParameter(strSPEC, _T("PER"), &pSPECSettings->PER);
	m_pParser->GetTestParameter(strSPEC, _T("RSSI_L"), &pSPECSettings->RssiL);
	m_pParser->GetTestParameter(strSPEC, _T("RSSI_H"), &pSPECSettings->RssiH);

	//Print Title
	if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Frequency: %.f, Data Rate: %s, Bandwidth: %s, Antenna: %s, Tx Power: %2.1f\r\n"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel), pRFSettings->TxPower);
	strTestItem.Format(_T("RX_%.f_%s_%s_%s"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel));

/*vDUT/ Equipment setRx*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!vDUT_SetRx(pRFSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_SetRx failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_SetRx failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!setupVSG(pRFSettings, pszModFile, frameCnt))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("setupVSG failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("setupVSG failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	do 
	{
		do 
		{
			/*Reset*/
			memset(m_szRespMsg, 0, STRING_LENS_4096);
			if(!vDUT_ResetRx(m_szRespMsg, STRING_LENS_4096))
			{
				m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_ResetRx failed, errMsg: %s"), m_szRespMsg);
				SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_ResetRx failed, errMsg: %s"), m_szRespMsg);

				goto go_Stop;
			}

			/*Generate Packet*/
			memset(m_szRespMsg, 0, STRING_LENS_4096);
			if(!m_EquipCtrl->StartGenerate(frameCnt, m_szRespMsg, STRING_LENS_4096))
			{
				m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("StartGenerate failed, errMsg: %s"), m_szRespMsg);
				SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("StartGenerate failed, errMsg: %s"), m_szRespMsg);

				goto go_Stop;
			}

			/*Get ACK*/
			memset(m_szRespMsg, 0, STRING_LENS_4096);
			if(!vDUT_Start(RF_RX, pWIFIRx, m_szRespMsg, STRING_LENS_4096))
			{
				m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Start failed, errMsg: %s"), m_szRespMsg);
				SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_Start failed, errMsg: %s"), m_szRespMsg);

				goto go_Stop;
			}else
			{
				if(pWIFIRx->PER >= frameCnt) pWIFIRx->PER = frameCnt;
				pWIFIRx->PER = ((frameCnt-pWIFIRx->PER)/frameCnt)*100;
			}

			if((testResult=wifiCompareRxResult(strTestItem, (WIFI_RXRESULT*)pWIFIRx, pSPECSettings, ReMeasure))) break;

		} while (--ReMeasure >= 0);

go_Stop:
		if(m_CsvExport)
		{
			bIs2G = (pRFSettings->Channel <= 14);

			strTmp.Format(_T("%.fMHz_%s_%s_%s"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel));
			(bIs2G)?m_csv2gRxTitle.Add(strTmp):m_csv5gRxTitle.Add(strTmp);

			if(pWIFIRx->PER != NA_NUMBER)
			{
				strTmp.Format(_T("%-4.1f"), pWIFIRx->PER);
				(bIs2G)?m_csv2gPER.Add(strTmp):m_csv5gPER.Add(strTmp);
			}else (bIs2G)?m_csv2gPER.Add(_T("N/A")):m_csv5gPER.Add(_T("N/A"));
		}

		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!vDUT_Stop(RF_RX, NULL, m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Stop failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_Stop failed, errMsg: %s"), m_szRespMsg);

			goto go_End;
		}

	} while (false);

	rsRtn = testResult?STATUS_PASS:STATUS_SPEC_FAIL;
go_End:
	DEL_ITEM(pRFSettings)
	DEL_ITEM(pSPECSettings)
	DEL_ITEM(pWIFIRx);
	DEL_ARRAY(pszModFile)
	return rsRtn;
}

RUNSTATUS CTestItems::WIFI_11AC_RX_SWEEP(CString strParameter, CString strSPEC, CString strOptions)
{
	RUNSTATUS rsRtn = STATUS_FUNCTION_ERROR;
	RF_SETTINGS *pRFSettings = new RF_SETTINGS;
	SPEC_SETTINGS *pSPECSettings = new SPEC_SETTINGS();
	WIFI_RXRESULT *pWIFIRx = new WIFI_RXRESULT();
	TCHAR *pszModFile = new TCHAR[MAX_PATH];

	int ReMeasure = 0, frameCnt = 0;;
	//double Pathloss;
	bool testResult = false;
	CString strTestItem = _T("");

	memset(pszModFile, 0, MAX_PATH);

	/*Parser*/
	m_pParser->GetTestParameter(strOptions, _T("REMEASURE"), &ReMeasure);

	if(!checkEquipCtrl(_T("WIFI_11AC_RX_VERIFY_PER"))) goto go_End;	

	/*vDUT_Init*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!(vDUT_Init(m_pDutSettings, m_szRespMsg, STRING_LENS_4096)))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("WIFI_11AC_TX_VERIFY_ALL"), _T("vDUT_Init failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}

	//Parameter
	memset(pRFSettings, 0, sizeof(RF_SETTINGS));
	if(!(
		(m_pParser->GetTestParameter(strParameter, _T("ANALYZE"), &pRFSettings->AnalyzeType)) && (m_pParser->GetTestParameter(strParameter, _T("CH"), &pRFSettings->Channel)) &&
		(m_pParser->GetTestParameter(strParameter, _T("RATE"), &pRFSettings->DataRate)) && (m_pParser->GetTestParameter(strParameter, _T("BW"), &pRFSettings->BandWidth)) &&
		(m_pParser->GetTestParameter(strParameter, _T("ANT"), &pRFSettings->AntSel)) && (m_pParser->GetTestParameter(strParameter, _T("POWER"), &pRFSettings->RxPower)) &&
		(m_pParser->GetTestParameter(strParameter, _T("PACKET"), &frameCnt)) && (m_pParser->GetTestParameter(strParameter, _T("WAVEFORM"), pszModFile, MAX_PATH)) &&
		(m_pParser->GetPathlossByCH(pRFSettings->Channel, pRFSettings->AntSel, &pRFSettings->Pathloss, pRFSettings->AllPathloss))
		))	
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Test parameter incorrect"));
		SetErrorReport(_T(""), _T("WIFI_11AC_RX_VERIFY_PER"), _T("Test parameter incorrect"));

		goto go_End;
	}

	//SPEC
	m_pParser->GetTestParameter(strSPEC, _T("PER"), &pSPECSettings->PER);
	m_pParser->GetTestParameter(strSPEC, _T("RSSI_L"), &pSPECSettings->RssiL);
	m_pParser->GetTestParameter(strSPEC, _T("RSSI_H"), &pSPECSettings->RssiH);

	//Print Title
	if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Frequency: %.f, Data Rate: %s, Bandwidth: %s, Antenna: %s, Tx Power: %2.1f"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel), pRFSettings->TxPower);
	strTestItem.Format(_T("RX_%.f_%s_%s_%s"), CConverter::Channel2Frequency(pRFSettings->Channel)/1000000, CConverter::DataRate2String(pRFSettings->DataRate), CConverter::Bandwidth2String(pRFSettings->BandWidth), CConverter::Antenna2String(pRFSettings->AntSel));

	/*vDUT/ Equipment setTx*/
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!vDUT_SetRx(pRFSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_SetRx failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_SetRx failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}	

	for(double swPower=pRFSettings->SweepPower[0] ; swPower>=pRFSettings->SweepPower[1] ; swPower-=pRFSettings->SweepStep)
	{
		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!setupVSG(pRFSettings, pszModFile, frameCnt))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("setupVSG failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("setupVSG failed, errMsg: %s"), m_szRespMsg);

			goto go_Stop;
		}

		/*Reset*/
		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!vDUT_ResetRx(m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_ResetRx failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_ResetRx failed, errMsg: %s"), m_szRespMsg);

			goto go_Stop;
		}

		/*Generate Packet*/
		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!m_EquipCtrl->StartGenerate(frameCnt, m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("StartGenerate failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("StartGenerate failed, errMsg: %s"), m_szRespMsg);

			goto go_Stop;
		}

		/*Get ACK*/
		memset(m_szRespMsg, 0, STRING_LENS_4096);
		if(!vDUT_Start(RF_RX, pWIFIRx, m_szRespMsg, STRING_LENS_4096))
		{
			m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Start failed, errMsg: %s"), m_szRespMsg);
			SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_Start failed, errMsg: %s"), m_szRespMsg);

			goto go_Stop;
		}else
		{
			if(pWIFIRx->PER >= frameCnt) pWIFIRx->PER = frameCnt;
			pWIFIRx->PER = ((frameCnt-pWIFIRx->PER)/frameCnt)*100;
		}
		
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("%-25s (%3.2fdBm): %-3.2f \t(    , %-4.1f)"), _T("PER"), pRFSettings->RxPower, pWIFIRx->PER, (pSPECSettings->PER==NA_NUMBER)?0:pSPECSettings->PER);

		if((pSPECSettings->PER != NA_NUMBER) && (pWIFIRx->PER > pSPECSettings->PER)) break;		
	}

go_Stop:
	memset(m_szRespMsg, 0, STRING_LENS_4096);
	if(!vDUT_Stop(RF_RX, NULL, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("vDUT_Stop failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("vDUT_Stop failed, errMsg: %s"), m_szRespMsg);

		goto go_End;
	}
	
go_End:
	DEL_ITEM(pRFSettings)
	DEL_ITEM(pSPECSettings)
	DEL_ITEM(pWIFIRx);
	DEL_ARRAY(pszModFile)
	return rsRtn;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

void CTestItems::SetErrorReport(LPSTR lpErrorCode, LPSTR lpTestItem, TCHAR *szFormat, ...)
{
	TCHAR *pBuffer = NULL;
	if(!m_pTestResult->TestResult) goto go_End;

	pBuffer = new TCHAR[STRING_LENS_256];
	memset(pBuffer, 0, STRING_LENS_256);

	va_list marker;
	va_start(marker, szFormat);
	vsprintf_s(pBuffer, STRING_LENS_256, szFormat, marker);
	va_end(marker);

	m_pTestResult->TestResult = false;
	strcpy_s(m_pTestResult->ErrorCode, STRING_LENS_32, lpErrorCode);
	strcpy_s(m_pTestResult->ErrorItem, STRING_LENS_64, lpTestItem);
	if(strlen(pBuffer) < STRING_LENS_256) strcpy_s(m_pTestResult->ErrorMessage, STRING_LENS_256, pBuffer);

go_End:
	DEL_ARRAY(pBuffer)
}

void CTestItems::ShowIQParameter(IQANALYZE_SETTING *pIQAnalySetting)
{
	CString strValue, strLog, strTmp;

/*802.11B*/
	if(pIQAnalySetting->wifi_11b_eq_taps == 1) strValue = _T("Off");
	else strValue.Format(_T("IQV_EQ_%d_TAPS"), pIQAnalySetting->wifi_11b_eq_taps);
	strLog.Format(_T("> 802.11b\r\n  %-30s : %d (%s)\r\n  %-30s : %d (%s)\r\n  %-30s : %d (%s)\r\n"), _T("eq_taps"), pIQAnalySetting->wifi_11b_eq_taps, strValue,
		_T("DCremove11b_flag"), pIQAnalySetting->wifi_11b_DCremove11b_flag, (pIQAnalySetting->wifi_11b_DCremove11b_flag)?_T("On"):_T("Off"),
		_T("method_11b"), pIQAnalySetting->wifi_11b_method_11b, (pIQAnalySetting->wifi_11b_method_11b == 1)?_T("Standard Tx ACC"):_T("RMS Error Vector"));
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, strLog.GetBuffer());

/*802.11AG*/
	strLog = _T("> 802.11ag\r\n  ");
	if(pIQAnalySetting->wifi_11ag_ph_corr_mode == 1) strValue = _T("Off");
	else if(pIQAnalySetting->wifi_11ag_ph_corr_mode == 2) strValue = _T("Sym by Sym");
	else if(pIQAnalySetting->wifi_11ag_ph_corr_mode == 3) strValue = _T("Moving Avg");
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("ph_corr_mode"), pIQAnalySetting->wifi_11ag_ph_corr_mode, strValue);
	strLog += strTmp;
	if(pIQAnalySetting->wifi_11ag_ch_estimate == 1) strValue = _T("Raw");
	else if(pIQAnalySetting->wifi_11ag_ch_estimate == 2) strValue = _T("2nd Order");
	else if(pIQAnalySetting->wifi_11ag_ch_estimate == 3) strValue = _T("Raw Full");
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("ch_estimate"), pIQAnalySetting->wifi_11ag_ch_estimate, strValue);
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("sym_tim_corr"), pIQAnalySetting->wifi_11ag_sym_tim_corr, (pIQAnalySetting->wifi_11ag_sym_tim_corr==1)?_T("Off"):_T("On"));
	strLog += strTmp;
	if(pIQAnalySetting->wifi_11ag_freq_sync == 1) strValue = _T("Short Train");
	else if(pIQAnalySetting->wifi_11ag_freq_sync == 2) strValue = _T("Long Train");
	else if(pIQAnalySetting->wifi_11ag_freq_sync == 3) strValue = _T("Full Packet");
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("freq_sync"), pIQAnalySetting->wifi_11ag_freq_sync, strValue);
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("ampl_track"), pIQAnalySetting->wifi_11ag_ampl_track, (pIQAnalySetting->wifi_11ag_ampl_track==1)?_T("Off"):_T("On"));
	strLog += strTmp;
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, strLog.GetBuffer());

/*802.11N*/
	strLog = _T("> 802.11n\r\n  ");
	strTmp.Format(_T("%-30s : %s\r\n  "), _T("type"), pIQAnalySetting->wifi_11n_type);
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %s\r\n  "), _T("mode"), pIQAnalySetting->wifi_11n_mode);
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enablePhaseCorr"), pIQAnalySetting->wifi_11n_enablePhaseCorr, (pIQAnalySetting->wifi_11n_enablePhaseCorr==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enableSymTimingCorr"), pIQAnalySetting->wifi_11n_enableSymTimingCorr, (pIQAnalySetting->wifi_11n_enableSymTimingCorr==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enableAmplitudeTracking"), pIQAnalySetting->wifi_11n_enableAmplitudeTracking, (pIQAnalySetting->wifi_11n_enableAmplitudeTracking==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("decodePSDU"), pIQAnalySetting->wifi_11n_decodePSDU, (pIQAnalySetting->wifi_11n_decodePSDU==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enableFullPacketChannelEst"), pIQAnalySetting->wifi_11n_enableFullPacketChannelEst, (pIQAnalySetting->wifi_11n_enableFullPacketChannelEst==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	if(pIQAnalySetting->wifi_11n_packetFormat == 0) strValue = _T("Auto-detect Mode");
	else if(pIQAnalySetting->wifi_11n_packetFormat == 1) strValue = _T("Mixed Format");
	else if(pIQAnalySetting->wifi_11n_packetFormat == 2) strValue = _T("Greenfield Format");
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("packetFormat"), pIQAnalySetting->wifi_11n_packetFormat, strValue);
	strLog += strTmp;
	if(pIQAnalySetting->wifi_11n_frequencyCorr == 2) strValue = _T("Long Training Field");
	else if(pIQAnalySetting->wifi_11n_frequencyCorr == 3) strValue = _T("Full Packet");
	else if(pIQAnalySetting->wifi_11n_frequencyCorr == 4) strValue = _T("LTF+SIG Field");
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("frequencyCorr"), pIQAnalySetting->wifi_11n_frequencyCorr, strValue);
	strLog += strTmp;
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, strLog.GetBuffer());

/*802.11AC*/
	strLog = _T("> 802.11ac\r\n  ");
	strTmp.Format(_T("%-30s : %s\r\n  "), _T("mode"), pIQAnalySetting->wifi_11ac_mode);
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enablePhaseCorr"), pIQAnalySetting->wifi_11ac_enablePhaseCorr, (pIQAnalySetting->wifi_11ac_enablePhaseCorr==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enableSymTimingCorr"), pIQAnalySetting->wifi_11ac_enableSymTimingCorr, (pIQAnalySetting->wifi_11ac_enableSymTimingCorr==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enableAmplitudeTracking"), pIQAnalySetting->wifi_11ac_enableAmplitudeTracking, (pIQAnalySetting->wifi_11ac_enableAmplitudeTracking==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("decodePSDU"), pIQAnalySetting->wifi_11ac_decodePSDU, (pIQAnalySetting->wifi_11ac_decodePSDU==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("enableFullPacketChannelEst"), pIQAnalySetting->wifi_11ac_enableFullPacketChannelEst, (pIQAnalySetting->wifi_11ac_enableFullPacketChannelEst==1)?_T("Enabled"):_T("Disabled"));
	strLog += strTmp;
	if(pIQAnalySetting->wifi_11ac_frequencyCorr == 1) strValue = _T("Frequency correction on short legacy training fields");
	else if(pIQAnalySetting->wifi_11ac_frequencyCorr == 2) strValue = _T("Frequency correction on long legacy training fields");
	else if(pIQAnalySetting->wifi_11ac_frequencyCorr == 3) strValue = _T("Frequency correction based on whole packet");
	else if(pIQAnalySetting->wifi_11ac_frequencyCorr == 4) strValue = _T("Frequency correction on signal fields (legacy and HT) in addition to long training fields");
	strTmp.Format(_T("%-30s : %d (%s)\r\n  "), _T("frequencyCorr"), pIQAnalySetting->wifi_11ac_frequencyCorr, strValue);
	strLog += strTmp;
	if(pIQAnalySetting->wifi_11ac_packetFormat == 0) strValue = _T("Auto-detect Mode");
	else if(pIQAnalySetting->wifi_11ac_packetFormat == 1) strValue = _T("Mixed Format");
	else if(pIQAnalySetting->wifi_11ac_packetFormat == 2) strValue = _T("Greenfield Format");
	else if(pIQAnalySetting->wifi_11ac_packetFormat == 3) strValue = _T("Legacy Format");
	else if(pIQAnalySetting->wifi_11ac_packetFormat == 4) strValue = _T("VHT Format");
	strTmp.Format(_T("%-30s : %d (%s)  "), _T("packetFormat"), pIQAnalySetting->wifi_11ac_packetFormat, strValue);
	strLog += strTmp;
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, strLog.GetBuffer());
}

bool CTestItems::Init_NIDevice(CString strParameter)
{
	bool bRtn = false;
	memset(m_szRespMsg, 0, STRING_LENS_4096);

	NIDEV_SETTINGS *pNIDevSettings = new NIDEV_SETTINGS;
	memset(pNIDevSettings, 0, sizeof(NIDEV_SETTINGS));

	if((m_pParser->GetTestParameter(strParameter, _T("IP"), pNIDevSettings->IPAddr, IP_ADDRESS)) &&
		(m_pParser->GetTestParameter(strParameter, _T("PORT"), &pNIDevSettings->Port))
		)
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Input parameter incorrect, Please check."));
		SetErrorReport(_T(""), _T("ATC_CONNECT_TESTER"), _T("Input parameter incorrect, Param: '%s'"), strParameter);

		goto go_End;
	}

	//Create CLPIQ
	if(!(!m_EquipCtrl && (m_EquipCtrl=(CNI_GIT*) new CNI_GIT())))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Memory allocate for NI-GIT failed"));
		SetErrorReport(_T(""), _T("ATC_CONNECT_TESTER"), _T("Memory allocate for NI-GIT failed"));

		goto go_End;	
	}

	((CNI_GIT*)m_EquipCtrl)->InitializeEquip(m_pLogger);

	if(!((CNI_GIT*)m_EquipCtrl)->ConnectDevice((void*)pNIDevSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("ConnectDevice failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("ATC_CONNECT_TESTER"), _T("ConnectDevice failed, errMsg: %s"), m_szRespMsg);

		goto go_End;	
	}

	bRtn = true;
go_End:
	return bRtn;
}

bool CTestItems::Init_IQDevice(CString strParameter)
{
	bool bRtn = false;
	memset(m_szRespMsg, 0, STRING_LENS_4096);

	IQDEV_SETTINGS *pIQDevSettings = new IQDEV_SETTINGS;
	memset(pIQDevSettings, 0, sizeof(IQDEV_SETTINGS));

	//Parser
	if(!(m_pParser->GetTestParameter(strParameter, _T("LOCKTIMEOUT"), &pIQDevSettings->lockTimeoutSec))) pIQDevSettings->lockTimeoutSec = 300; //Set default (sec)
	if(!(
		(m_pParser->GetTestParameter(strParameter, _T("VSAVSG"), &pIQDevSettings->MeasurePort)) &&
		(m_pParser->GetTestParameter(strParameter, _T("IP"), pIQDevSettings->IPAddr, IP_ADDRESS, &pIQDevSettings->IQxelConnectionType)))
		)
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Input parameter incorrect, Please check."));
		SetErrorReport(_T(""), _T("ATC_CONNECT_TESTER"), _T("Input parameter incorrect, Param: '%s'"), strParameter);

		goto go_End;
	}
	
	pIQDevSettings->rfType = m_rfMeasurer;

	//Create CLPIQ
	if(!m_EquipCtrl)
	{
		if(!(m_EquipCtrl=(CLPIQ*) new CLPIQ()))
		{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("Memory allocate for LitepointIQ failed"));
		SetErrorReport(_T(""), _T("ATC_CONNECT_TESTER"), _T("IMemory allocate for LitepointIQ failed"));

		goto go_End;	
		}
	}
	
	((CLPIQ*)m_EquipCtrl)->InitializeEquip(m_pLogger);

	if(!((CLPIQ*)m_EquipCtrl)->ConnectDevice((void*)pIQDevSettings, m_szRespMsg, STRING_LENS_4096))
	{
		m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("ConnectDevice failed, errMsg: %s"), m_szRespMsg);
		SetErrorReport(_T(""), _T("ATC_CONNECT_TESTER"), _T("ConnectDevice failed, errMsg: %s"), m_szRespMsg);

		goto go_End;	
	}
	
	//Show Inoformation
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("IQmeasure : %s"), ((CLPIQ*)m_EquipCtrl)->m_DeviceInfo.szIQmeasure);
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Tester SN : %s"), ((CLPIQ*)m_EquipCtrl)->m_DeviceInfo.szSN);
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Model : %s"), ((CLPIQ*)m_EquipCtrl)->m_DeviceInfo.szModel);
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("FW revision : %s"), ((CLPIQ*)m_EquipCtrl)->m_DeviceInfo.szFWversion);
	m_pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("HW version : %s"), ((CLPIQ*)m_EquipCtrl)->m_DeviceInfo.szHWversion);

	bRtn = true;
go_End:
	DEL_ITEM(pIQDevSettings)
	return bRtn;
}

bool CTestItems::checkEquipCtrl(CString strTestItem)
{
	bool bRtn = false;

	if(m_EquipCtrl)
	{
		if((m_rfMeasurer == RF_IQXEL_80) || (m_rfMeasurer == RF_IQXEL_M8) || (m_rfMeasurer == RF_IQXEL_M2W)) bRtn = ((CLPIQ*)m_EquipCtrl)->m_Initialized;
		else if(m_rfMeasurer == RF_NI) bRtn = ((CNI_GIT*)m_EquipCtrl)->m_Initialized;		
	}

	if(!bRtn)
	{
		SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("EquipCtrl isn't initialize"));
		if(m_pLogger) m_pLogger->OutputLog(TYPE_NORMAL, COLOR_RED, _T("EquipCtrl isn't initialize"));
	}
	return bRtn;
}

bool CTestItems::setupVSG(RF_SETTINGS *pRFSettings, LPSTR szModFile, int frameCnt)
{
	bool bRtn = false;
	void *pVSGSetting = NULL;

	if(m_EquipCtrl)
	{
		if(((m_rfMeasurer == RF_IQXEL_80) || (m_rfMeasurer == RF_IQXEL_M8) || (m_rfMeasurer == RF_IQXEL_M2W)) && (((CLPIQ*)m_EquipCtrl)->m_Initialized))
		{
			pVSGSetting = new IQVSG_SETTINGS;
			memset(pVSGSetting, 0, sizeof(IQVSG_SETTINGS));

			((IQVSG_SETTINGS*)pVSGSetting)->Frequency = (double)CConverter::Channel2Frequency(pRFSettings->Channel);
			((IQVSG_SETTINGS*)pVSGSetting)->rfPowerLeveldBm = pRFSettings->RxPower+pRFSettings->Pathloss;
			((IQVSG_SETTINGS*)pVSGSetting)->frameCnt = frameCnt;

			if(strstr(szModFile, _T("/user/"))) sprintf_s(((IQVSG_SETTINGS*)pVSGSetting)->szModFile, MAX_PATH, _T("//user//%s"), szModFile);
			else
			{
				sprintf_s(((IQVSG_SETTINGS*)pVSGSetting)->szModFile, MAX_PATH, _T("%s\\Mod\\%s"), CCommFunc::m_ModulePath, szModFile);

				if(!(bRtn=CCommFunc::FileExist(((IQVSG_SETTINGS*)pVSGSetting)->szModFile)))
				{
					sprintf_s(m_szRespMsg, STRING_LENS_2048, _T("The waveform file doesn't exist, file: %s"), ((IQVSG_SETTINGS*)pVSGSetting)->szModFile);
					goto go_End;
				}	
			}

			if(!(m_EquipCtrl->SetupVSG(pVSGSetting, m_szRespMsg, STRING_LENS_4096))) goto go_End;
			else bRtn = true;

		}else if(m_rfMeasurer == RF_NI)
		{

		}
	}

go_End:
	DEL_ITEM(pVSGSetting);
	return bRtn;
}

bool CTestItems::wifiCompareTxResult(CString strTestItem, WIFI_TXRESULT *pWifiTxResult, SPEC_SETTINGS *pSPECSettings, int MeasureCnt)
{
	bool bPower = true, bEVM = true, bFreqError = true, bMask = true, bLoLeakage = true;
	CString strTmp;

	//EVM
	if(pSPECSettings->EVM != NA_NUMBER)
	{
		if(!(bEVM = (pWifiTxResult->evmAll != NA_NUMBER) && (pWifiTxResult->evmAll <= pSPECSettings->EVM)))
		{
			if(MeasureCnt <= 0) SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("Don't match the pass criteria, EVM: %3.3f dBm (%3.1f ~ )"), pWifiTxResult->evmAll, pSPECSettings->EVM);			
			else 
			{
				printf(_T("... EVM Retry. %-3.2f dB\n"), pWifiTxResult->evmAll);
				goto go_End;
			}
		}
	}

	//Freq Error
	if(pSPECSettings->FreqError != NA_NUMBER)
	{
		if(!(bFreqError = (pWifiTxResult->clockErr != NA_NUMBER) && (fabs(pWifiTxResult->clockErr) <= pSPECSettings->FreqError)))
		{
			if(MeasureCnt <= 0) SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("Don't match the pass criteria, FreqErr: %3.3f ppm (%3.1f ~ )"), pWifiTxResult->clockErr, pSPECSettings->FreqError);	
			else
			{
				printf(_T("... FreqError Retry. %-3.2f ppm\n"), pWifiTxResult->clockErr);
				goto go_End;
			}
		}
	}

	//Tx Power
	if((pSPECSettings->PowerH != NA_NUMBER) && (pSPECSettings->PowerL != NA_NUMBER))
	{
		if(!(bPower = (pWifiTxResult->rmsPower != NA_NUMBER) && ((pWifiTxResult->rmsPower <= pSPECSettings->PowerH) && (pWifiTxResult->rmsPower >= pSPECSettings->PowerL))))
		{
			if(MeasureCnt <= 0) SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("Don't match the pass criteria, Power: %3.3f dBm (%3.1f ~ %3.1f)"), pWifiTxResult->rmsPower, pSPECSettings->PowerH, pSPECSettings->PowerL);			
			else
			{
				printf(_T("... Power Retry. %-3.2f dBm\n"), pWifiTxResult->rmsPower);
				goto go_End;
			}
		}	
	}	

	//Spectrum Violation
	if(pSPECSettings->SpectrumViolation != NA_NUMBER)
	{
		if(!(bMask=(pWifiTxResult->SpectrumMask != NA_NUMBER) && (pWifiTxResult->SpectrumMask <= pSPECSettings->SpectrumViolation)))
		{
			if(MeasureCnt <= 0) SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("Don't match the pass criteria, Spectrum Violation: %3.3f %% (%3.1f ~ )"), pWifiTxResult->SpectrumMask, pSPECSettings->SpectrumViolation);	
			else
			{
				printf(_T("... SpectrumViolation Retry. %-3.2f\n"), pWifiTxResult->SpectrumMask);
				goto go_End;
			}
		}
	}

	//LoLeakage
	if(pSPECSettings->LoLeakage != NA_NUMBER)
	{
		if(!(bLoLeakage=(pWifiTxResult->loLeakageDb != NA_NUMBER) && (pWifiTxResult->loLeakageDb <= pSPECSettings->LoLeakage)))
		{
			if(MeasureCnt <= 0) SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("Don't match the pass criteria, LoLeakage: %3.3f %% (%3.1f ~ )"), pWifiTxResult->loLeakageDb, pSPECSettings->LoLeakage);	
			else 
			{
				printf(_T("... LoLeakage Retry. %-3.2f\n"), pWifiTxResult->loLeakageDb);
				goto go_End;
			}
		}
	}

	if(m_pLogger)
	{
		if((pSPECSettings->PowerH != NA_NUMBER) && (pSPECSettings->PowerL != NA_NUMBER)) 
			m_pLogger->OutputLog(TYPE_NORMAL, bPower?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f dBm \t (%3.1f ~ %3.1f) \t %s"), _T("Power"), _T(" "), pWifiTxResult->rmsPower, pSPECSettings->PowerH, pSPECSettings->PowerL, bPower?_T("<-- pass"):_T("<-- fail"));
		else m_pLogger->OutputLog(TYPE_NORMAL, bPower?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f dBm \t (%-4s ~ %-4s) \t %s"), _T("Power"), _T(" "), pWifiTxResult->rmsPower, _T("N/A"), _T("N/A"), bPower?_T("<-- pass"):_T("<-- fail"));

		if(pSPECSettings->EVM != NA_NUMBER) 
			m_pLogger->OutputLog(TYPE_NORMAL, bEVM?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f dB  \t (%3.1f ~ ) \t %s"), _T("EVM"), _T(" "), pWifiTxResult->evmAll, pSPECSettings->EVM, bEVM?_T("<-- pass"):_T("<-- fail"));
		else m_pLogger->OutputLog(TYPE_NORMAL, bEVM?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f dB  \t (%-4s ~ ) \t %s"), _T("EVM"), _T(" "), pWifiTxResult->evmAll, _T("N/A"), bEVM?_T("<-- pass"):_T("<-- fail"));

		if(pSPECSettings->FreqError != NA_NUMBER) 
			m_pLogger->OutputLog(TYPE_NORMAL, bFreqError?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f ppm \t (%3.1f ~ ) \t %s"), _T("Freq Error"), _T(" "), pWifiTxResult->clockErr, pSPECSettings->FreqError, bFreqError?_T("<-- pass"):_T("<-- fail"));
		else m_pLogger->OutputLog(TYPE_NORMAL, bFreqError?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f ppm \t (%-4s ~ ) \t %s"), _T("Freq Error"), _T(" "), pWifiTxResult->clockErr, __T("N/A"), bFreqError?_T("<-- pass"):_T("<-- fail"));

		if(pSPECSettings->SpectrumViolation != NA_NUMBER) 
			m_pLogger->OutputLog(TYPE_NORMAL, bMask?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f %%   \t (%3.2f ~ ) \t %s"), _T("Spectrum Mask"), _T(" "), pWifiTxResult->SpectrumMask, pSPECSettings->SpectrumViolation, bMask?_T("<-- pass"):_T("<-- fail"));
		else m_pLogger->OutputLog(TYPE_NORMAL, bMask?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f %%   \t (%-4s ~ ) \t %s"), _T("Spectrum Mask"), _T(" "), pWifiTxResult->SpectrumMask, _T("N/A"), bMask?_T("<-- pass"):_T("<-- fail"));

		if(pSPECSettings->LoLeakage != NA_NUMBER)
			m_pLogger->OutputLog(TYPE_NORMAL, bMask?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f %%   \t (%3.2f ~ ) \t %s"), _T("LO Leakage"), _T(" "), pWifiTxResult->loLeakageDb, pSPECSettings->LoLeakage, bLoLeakage?_T("<-- pass"):_T("<-- fail"));
		else m_pLogger->OutputLog(TYPE_NORMAL, bMask?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %3.3f %%   \t (%-4s ~ ) \t %s"), _T("LO Leakage"), _T(" "), pWifiTxResult->loLeakageDb, _T("N/A"), bMask?_T("<-- pass"):_T("<-- fail"));
	}

go_End:
	return (bPower && bEVM && bFreqError && bMask);
}

bool CTestItems::wifiCompareRxResult(CString strTestItem, WIFI_RXRESULT *pWifiRxResult, SPEC_SETTINGS *pSPECSettings, int MeasureCnt)
{
	bool bPER = true, bRSSI = true;

	/*PER*/
	if(pSPECSettings->PER != NA_NUMBER)
	{
		if(!(bPER=((pWifiRxResult->PER != NA_NUMBER) && (pWifiRxResult->PER <= pSPECSettings->PER))))
		{
			if(MeasureCnt <= 0) SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("Don't match the pass criteria, PER: %2.3f (%2.1f ~ 0.0)"), pWifiRxResult->PER, pSPECSettings->PER);	
			else
			{
				printf(_T("... PER Retry. %-4.1f\n"), pWifiRxResult->PER);
				goto go_End;
			}
		}
	}

	//RSSI
	if((pSPECSettings->RssiH != NA_NUMBER) && (pSPECSettings->RssiL != NA_NUMBER))
	{
		if(!(bRSSI=((pWifiRxResult->RSSI != NA_NUMBER) &&
			((pWifiRxResult->RSSI >= pSPECSettings->RssiL) && (pWifiRxResult->RSSI <= pSPECSettings->RssiH))
			)))
		{
			if(MeasureCnt <= 0) SetErrorReport(_T(""), strTestItem.GetBuffer(), _T("Don't match the pass criteria, RSSI: %2.3f (%2.1f ~ %2.1f)"), pWifiRxResult->RSSI, pSPECSettings->RssiH, pSPECSettings->RssiL);	
			else
			{
				printf(_T("... RSSI Retry. %-3.2f dBm\n"), pWifiRxResult->RSSI);
				goto go_End;
			}
		}
	}

	if(m_pLogger)
	{                                                                                                      
		if(pSPECSettings->PER != NA_NUMBER) 
			m_pLogger->OutputLog(TYPE_NORMAL, bPER?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %2.3f \t\t (%2.1f ~ 0.0) \t %s"), _T("PER"), _T(" "), pWifiRxResult->PER, pSPECSettings->PER, bPER?_T("<-- pass"):_T("<-- fail"));
		else m_pLogger->OutputLog(TYPE_NORMAL, bPER?COLOR_GREEN:COLOR_RED, _T("%15s %10s %2.3f \t\t (%-4s ~ 0.0) \t %s"), _T("PER"), _T(" "), pWifiRxResult->PER, _T("N/A"), bPER?_T("<-- pass"):_T("<-- fail"));

		if((pSPECSettings->RssiH != NA_NUMBER) && (pSPECSettings->RssiL != NA_NUMBER)) 
			m_pLogger->OutputLog(TYPE_NORMAL, bRSSI?COLOR_GREEN:COLOR_RED, _T("%-15s %10s %2.3f \t\t (%2.1f ~ %2.1f) \t %s"), _T("RSSI"), _T(" "), pWifiRxResult->RSSI, pSPECSettings->RssiH, pSPECSettings->RssiL, bRSSI?_T("<-- pass"):_T("<-- fail"));
		else 
		{
			if(pWifiRxResult->RSSI != NA_NUMBER) m_pLogger->OutputLog(TYPE_NORMAL, bRSSI?COLOR_GREEN:COLOR_RED, _T("%15s %10s %2.3f \t\t (%4s ~ %4s) \t %s"), _T("RSSI"), _T(" "), pWifiRxResult->RSSI, _T("N/A"), _T("N/A"), bRSSI?_T("<-- pass"):_T("<-- fail"));
		}
	}

go_End:
	return (bPER && bRSSI);
}