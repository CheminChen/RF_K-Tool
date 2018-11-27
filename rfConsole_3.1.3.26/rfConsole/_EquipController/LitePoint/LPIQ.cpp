#include "StdAfx.h"
#include "LPIQ.h"

#include "LP_Utility.h"

CLPIQ::CLPIQ(void)
{
	m_phSpectrumMask = NULL;
	m_rfType = RF_IQXEL_80;
	m_MeasurePort = PORT_LEFT;
	m_samplingRate = 160e6;
	m_Initialized = false;
	m_bVSGOnOff = false;

	m_numSegm = 1;

	memset(&m_DeviceInfo, 0, sizeof(INFO));
}

CLPIQ::~CLPIQ(void)
{
}

void CLPIQ::InitializeEquip(CLogger *pLogger)
{
	m_pLogger = pLogger;

	defineSpectrumMaskLimit();
}

void CLPIQ::TerminateEquip()
{
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Term()"));
#endif
	LP_Term();	
}

bool CLPIQ::ReleaseLock(LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int errStatus;

#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Async_ReleaseTesterLock()"));
#endif
	if((errStatus=LP_Async_ReleaseTesterLock()) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Async_ReleaseTesterLock failed, errMsg: "));
		goto go_Error;
	}

	bRtn = true;
	return bRtn;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return bRtn;
}

bool CLPIQ::GetLock(LPSTR szRespMsg, int nLens)
{
	bool bRtn = false, isGot;
	int errStatus;

#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Async_GetTesterLock()"));
#endif
	if((errStatus=LP_Async_GetTesterLock(&isGot)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Async_GetTesterLock failed, errMsg: "));
		goto go_Error;
	}else
	{
		if(isGot) bRtn = true;
		else strcpy_s(szRespMsg, nLens, _T("Get tester lock failed"));
	}

	return bRtn;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return bRtn;
}

bool CLPIQ::ConnectDevice(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int errStatus = ERR_OK, DUT_ID, numModsPerIP[4];
	IQDEV_SETTINGS *iqSettings = NULL;

	if(dataStruct) iqSettings = (IQDEV_SETTINGS*)dataStruct;
	else
	{
		strcpy_s(szRespMsg, nLens, _T("dataStruct is null"));
		goto go_Error;
	}

	if((errStatus=LP_Init(IQTYPE_XEL, 1)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Init failed, errMsg: "));
		goto go_Error;
	}

	DUT_ID = iqSettings->MeasurePort-1 + ((iqSettings->IQxelConnectionType-1)*4);
	if((errStatus=LP_Async_Initialize(DUT_ID, iqSettings->lockTimeoutSec)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Async_Initialize failed, errMsg: "));
		goto go_Error;
	}

	LP_InitTesterN(iqSettings->IPAddr, 1, &numModsPerIP[0]);
	/*
	if((errStatus=LP_InitTesterN(iqSettings->IPAddr, 1, &numModsPerIP[0])) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_InitTesterN failed, errMsg: "));
		goto go_Error;
	}*/

	int moduleGroup[2] = {iqSettings->IQxelConnectionType};
	if((errStatus=LP_SetTesterMode(UP_TO_80MHZ_SIGNAL, moduleGroup, 1)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_SetTesterMode failed, errMsg: "));
		goto go_Error;
	}

	if((errStatus=LP_Async_InitTxRxHandler()) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Async_InitTxRxHandler failed, errMsg: "));
		goto go_Error;
	}

	if(!(bRtn=LP_GetVersion(szRespMsg, nLens))) strcpy_s(szRespMsg, nLens, _T("LP_GetVersion failed"));
	else 
	{		
		m_rfType = iqSettings->rfType;
		m_MeasurePort = iqSettings->MeasurePort;

		if(iqSettings->rfType == RF_IQXEL_80) m_samplingRate = 160e6;
		else if(iqSettings->rfType == RF_IQXEL_M2W) m_samplingRate = 240e6;
		else m_samplingRate = 80e6;	

		ParserDeviceInfo(szRespMsg);
	}

	if((errStatus=LP_SetVsaTriggerTimeout(5.0)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_SetVsaTriggerTimeout failed, errMsg: "));
		goto go_Error;
	}

	m_Initialized = bRtn;
	return bRtn;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return bRtn;
}

void CLPIQ::ParserDeviceInfo(LPSTR lpInfoString)
{
	int nS, nE;
	CString strInfo(lpInfoString), strTmp;

	if((nS = strInfo.Find(_T("IQmeasure:"))) != -1)
	{
		nS += strlen(_T("IQmeasure:"));
		if((nE = strInfo.Find(_T("\n"), nS)) != -1)
		{
			strTmp = strInfo.Mid(nS, nE-nS);
			strcpy_s(m_DeviceInfo.szIQmeasure, STRING_LENS_64, strTmp.Trim());
		}
	}else strcpy_s(m_DeviceInfo.szIQmeasure, STRING_LENS_64, _T("N/A"));

	if((nS = strInfo.Find(_T("Tester 1 SN:"))) != -1)
	{
		nS += strlen(_T("Tester 1 SN:"));
		if((nE = strInfo.Find(_T("\n"), nS)) != -1)
		{
			strTmp = strInfo.Mid(nS, nE-nS);
			strcpy_s(m_DeviceInfo.szSN, STRING_LENS_64, strTmp.Trim());
		}
	}else strcpy_s(m_DeviceInfo.szSN, STRING_LENS_64, _T("N/A"));

	if((nS = strInfo.Find(_T("Model number:"))) != -1)
	{
		nS += strlen(_T("Model number:"));
		if((nE = strInfo.Find(_T("\n"), nS)) != -1)
		{
			strTmp = strInfo.Mid(nS, nE-nS);
			strcpy_s(m_DeviceInfo.szModel, STRING_LENS_64, strTmp.Trim());
		}
	}else strcpy_s(m_DeviceInfo.szModel, STRING_LENS_64, _T("N/A"));

	if((nS = strInfo.Find(_T("Firmware revision:"))) != -1)
	{
		nS += strlen(_T("Firmware revision:"));
		if((nE = strInfo.Find(_T("\n"), nS)) != -1)
		{
			strTmp = strInfo.Mid(nS, nE-nS);
			strcpy_s(m_DeviceInfo.szFWversion, STRING_LENS_64, strTmp.Trim());
		}
	}else strcpy_s(m_DeviceInfo.szFWversion, STRING_LENS_64, _T("N/A"));
	
	if((nS = strInfo.Find(_T("Tester 1 hardware version:"))) != -1)
	{
		nS += strlen(_T("Tester 1 hardware version:"));
		if((nE = strInfo.Find(_T("\n"), nS)) != -1)
		{
			strTmp = strInfo.Mid(nS, nE-nS);
			strcpy_s(m_DeviceInfo.szHWversion, STRING_LENS_64, strTmp.Trim());
		}
	}else strcpy_s(m_DeviceInfo.szHWversion, STRING_LENS_64, _T("N/A"));
}

bool CLPIQ::SetupVSG(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int errStatus = ERR_OK;
	IQVSG_SETTINGS *pIQVSGSettings = NULL;

	if(!m_Initialized)
	{
		strcpy_s(szRespMsg, nLens, _T("Equipment isn't initialize"));
		goto go_Error;
	}

	if(!dataStruct) 	
	{
		strcpy_s(szRespMsg, nLens, _T("dataStruct is null"));
		goto go_Error;
	}
	pIQVSGSettings = (IQVSG_SETTINGS*)dataStruct;
	memcpy_s(&m_IQVSGSettings, sizeof(IQVSG_SETTINGS), pIQVSGSettings, sizeof(IQVSG_SETTINGS));

	//LP_SetVsg
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_SetVsg(Frequency: %.0f, rfPowerLeveldBm: %.3f, MeasurePort: %d)"), pIQVSGSettings->Frequency, pIQVSGSettings->rfPowerLeveldBm, m_MeasurePort);
#endif
	if((errStatus=LP_SetVsg(m_IQVSGSettings.Frequency, m_IQVSGSettings.rfPowerLeveldBm, m_MeasurePort)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_SetVsg failed, errMsg: "));
		goto go_Error;
	}

	//LP_SetVsgModulation
	int loadInternalWaveform = (strstr(m_IQVSGSettings.szModFile, _T("/user/"))) ? 1:0;
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_SetVsgModulation(Mod: %s, loadInternalWaveform: %d)"), m_IQVSGSettings.szModFile, loadInternalWaveform);
#endif
	if((errStatus=LP_SetVsgModulation(m_IQVSGSettings.szModFile, loadInternalWaveform)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_SetVsgModulation failed, errMsg: "));
		goto go_Error;
	}

	//LP_EnableVsgRF
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_EnableVsgRF(1)"));
#endif
	if((errStatus=LP_EnableVsgRF(1)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_EnableVsgRF failed, errMsg: "));
		goto go_Error;
	}
	m_bVSGOnOff = true;

	//LP_SetFrameCnt
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_SetFrameCnt(1)"));
#endif
	if((errStatus=LP_SetFrameCnt(1)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_SetFrameCnt failed, errMsg: "));
		goto go_Error;
	}

	return bRtn=true;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return bRtn;
}

bool CLPIQ::SetupVSA(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int errStatus = ERR_OK;
	IQVSA_SETTINGS *pIQVSASettings = NULL;

	if(!m_Initialized)
	{
		strcpy_s(szRespMsg, nLens, _T("Equipment doesn't initialize"));
		goto go_Error;
	}

	if(!dataStruct) 	
	{
		strcpy_s(szRespMsg, nLens, _T("dataStruct is null"));
		goto go_Error;
	}
	pIQVSASettings = (IQVSA_SETTINGS*)dataStruct;
	memcpy_s(&m_IQVSASettings, sizeof(IQVSA_SETTINGS), pIQVSASettings, sizeof(IQVSA_SETTINGS));

	//Check for 11AC
	/*
	if((m_rfType == RF_IQ2010) && (m_IQVSASettings.AnalyzeType == STANDARD_802_11_AC))
	{
		strcpy_s(szRespMsg, nLens, _T("IQ2010 doesn't support 802.11AC"));
		goto go_Error;
	}*/

	if(m_IQVSASettings.AnalyzeType == STANDARD_BLUETOOTH)
	{
#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("LP_SetVsaBluetooth(Frequency: %.0f, rfAmplDb: %.3f, MeasurePort: %d, extAttenDb: 0, triggerLevelDb: %.1f, triggerPreTime: %f)"), pIQVSASettings->Frequency, pIQVSASettings->rfAmplDb, m_MeasurePort, pIQVSASettings->triggerLevelDb, pIQVSASettings->triggerPreTime);
#endif
		if(!(bRtn=((errStatus=LP_SetVsaBluetooth(pIQVSASettings->Frequency, pIQVSASettings->rfAmplDb, m_MeasurePort, pIQVSASettings->triggerLevelDb, pIQVSASettings->triggerPreTime)) == ERR_OK)))
		{
			strcpy_s(szRespMsg, nLens, _T("LP_SetVsaBluetooth failed, errMsg: "));
			goto go_Error;
		}
	}else
	{
#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("LP_SetVsa(Frequency: %.0f, rfAmplDb: %.3f, MeasurePort: %d, extAttenDb: 0, triggerLevelDb: %.1f, triggerPreTime: %f)"), pIQVSASettings->Frequency, pIQVSASettings->rfAmplDb, m_MeasurePort, pIQVSASettings->triggerLevelDb, pIQVSASettings->triggerPreTime);
#endif
		//if (!(bRtn=((errStatus = LP_SetVsa(5180e6, -10 + 10.0 - 1.0, PORT_1, 0, -25, 5e-6)) == ERR_OK)))
		if (!(bRtn=((errStatus = LP_SetVsa(pIQVSASettings->Frequency, pIQVSASettings->rfAmplDb, (int)m_MeasurePort, 0, pIQVSASettings->triggerLevelDb, pIQVSASettings->triggerPreTime)) == ERR_OK)))
		{
			strcpy_s(szRespMsg, nLens, _T("LP_SetVsa failed, errMsg: "));
			goto go_Error;
		}

		memset(m_IQAnalySetting.wifi_11ac_mode, 0, sizeof(m_IQAnalySetting.wifi_11ac_mode));
		strcpy_s(m_IQAnalySetting.wifi_11ac_mode, sizeof(m_IQAnalySetting.wifi_11ac_mode), pIQVSASettings->wifi_mode);
	}
	
	return bRtn;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return bRtn;
}

bool CLPIQ::StartMeasure(void *dataStruct, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	MEASURE_RESULT *pMeasureResult = NULL;
	int errStatus = ERR_OK;

	if(!m_Initialized)
	{
		strcpy_s(szRespMsg, nLens, _T("Equipment doesn't initialize"));
		goto go_Error;
	}

	if(!dataStruct) 	
	{
		strcpy_s(szRespMsg, nLens, _T("dataStruct is null"));
		goto go_Error;
	}
	pMeasureResult = (MEASURE_RESULT*)dataStruct;

	//LP_VsaDataCapture	
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Segms_VsaDataCapture(m_numSegm: %d, samplingTimeSecs: %f, triggerType: %d, m_samplingRate: %.0f, ht40Mode: %d)"), m_numSegm, m_IQVSASettings.samplingTimeSecs, m_IQVSASettings.triggerType, m_samplingRate, m_IQVSASettings.ht40Mode);
#endif
	if((errStatus=LP_Segms_VsaDataCapture(m_numSegm, m_IQVSASettings.samplingTimeSecs, m_IQVSASettings.triggerType, m_samplingRate, m_IQVSASettings.ht40Mode)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Segms_VsaDataCapture failed, errMsg: "));
		goto go_Error;
	}	

	//Analyze
	if((m_IQVSASettings.AnalyzeType == STANDARD_802_11_B) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_AG) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_N) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_AC) ||
		(m_IQVSASettings.AnalyzeType == STANDARD_802_11_B_CAL) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_AG_CAL) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_N_CAL) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_AC_CAL))
	{
		if((m_IQVSASettings.AnalyzeType == STANDARD_802_11_B) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_B_CAL)) 
			errStatus = Analyze80211b((WIFI_TXRESULT*)pMeasureResult->TestResult, szRespMsg, nLens);
		else if((m_IQVSASettings.AnalyzeType == STANDARD_802_11_AG) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_AG_CAL))
			errStatus = Analyze80211ag((WIFI_TXRESULT*)pMeasureResult->TestResult, szRespMsg, nLens);
		else if((m_IQVSASettings.AnalyzeType == STANDARD_802_11_N) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_N_CAL))
			errStatus = Analyze80211n((WIFI_TXRESULT*)pMeasureResult->TestResult, szRespMsg, nLens);
		else if((m_IQVSASettings.AnalyzeType == STANDARD_802_11_AC) || (m_IQVSASettings.AnalyzeType == STANDARD_802_11_AC_CAL))
			errStatus = Analyze80211ac((WIFI_TXRESULT*)pMeasureResult->TestResult, szRespMsg, nLens);
	}	
	else if(m_IQVSASettings.AnalyzeType == STANDARD_ZIGBEE) 
	{
		//errStatus = AnalyzeZigbee((ZB_RESULT*)pMeasureResult->TestResult);
	}else if(m_IQVSASettings.AnalyzeType == STANDARD_BLUETOOTH) 
	{}
	
	return (errStatus==ERR_OK);
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return bRtn;
}

bool CLPIQ::StartGenerate(int frameCnt, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int errStatus;

	if(!m_Initialized)
	{
		strcpy_s(szRespMsg, nLens, _T("Equipment doesn't initialize"));
		goto go_Error;
	}

	if(frameCnt == -1)
	{
		/*Stop VSG*/
#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("LP_EnableVsgRF(0)"));
#endif
		if((errStatus=LP_EnableVsgRF(0)) != ERR_OK)
		{
			strcpy_s(szRespMsg, nLens, _T("LP_EnableVsgRF(0) failed, errMsg: "));
			goto go_Error;
		}
		m_bVSGOnOff = true;

	}else if(frameCnt == 0)
	{
		/*Continuous*/
#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("LP_SetFrameCnt(counter: %d)"), frameCnt);
#endif
		if((errStatus=LP_SetFrameCnt(frameCnt)) != ERR_OK)
		{
			strcpy_s(szRespMsg, nLens, _T("LP_SetFrameCnt(0) failed, errMsg: "));
			goto go_Error;
		}
	}else
	{
#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("LP_SetFrameCnt(counter: %d)"), frameCnt);
#endif
		if((errStatus=LP_SetFrameCnt(frameCnt)) != ERR_OK)
		{
			strcpy_s(szRespMsg, nLens, _T("LP_SetFrameCnt failed, errMsg: "));
			goto go_Error;
		}

#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("Wait for LP_TxDone"));
#endif
		while(!(bRtn = ((errStatus = LP_TxDone()) == ERR_OK))) Sleep(100);
	}

	return (errStatus==ERR_OK);
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return bRtn;
}

bool CLPIQ::ReleaseCtrlDevice(LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int errStatus;

	if(!m_Initialized) strcpy_s(szRespMsg, nLens, _T("Equipment doesn't initialize"));
	else bRtn = true;
	
	return bRtn;
}

bool CLPIQ::DisconnectDevice(LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int errStatus;
	TCHAR szMsg[STRING_LENS_256] = {'\0'};

	if(!m_Initialized) strcpy_s(szRespMsg, nLens, _T("Equipment doesn't initialize"));
	else
	{
#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("LP_ConClose()"));
#endif
		if((errStatus = LP_ConClose()) != ERR_OK)
		{
			sprintf_s(szMsg, STRING_LENS_256, _T("Call LP_ConClose fail, Msg: "));
			goto go_Error;
		}

#if (_DEBUG)
		CCommFunc::OutputTracelog(_T("LP_Async_Term()"));
#endif
		if((errStatus = LP_Async_Term()) != ERR_OK)
		{
			sprintf_s(szMsg, STRING_LENS_256, _T("Call LP_Async_Term fail, Msg: "));
			goto go_Error;
		}

		bRtn = true;
		m_Initialized = false;
	}

	return bRtn;
go_Error:
	strcat_s(szMsg, STRING_LENS_256, LP_GetErrorString(errStatus));
	OutputDebugString(szMsg);
	
	return bRtn;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

void CLPIQ::SetAnalyzeSetting(IQANALYZE_SETTING *pIQAnalySetting)
{
	if(pIQAnalySetting) memcpy_s(&m_IQAnalySetting, sizeof(IQANALYZE_SETTING), pIQAnalySetting, sizeof(IQANALYZE_SETTING));
}

void CLPIQ::SetupSpectrumMask(HWND *phSpectrumMask)
{
	m_phSpectrumMask = phSpectrumMask;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

int CLPIQ::Analyze80211b(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens)
{
	int errStatus = ERR_OK;	
	double *bufferRealX = NULL, *bufferImagX = NULL, *bufferRealY=NULL, *bufferImagY=NULL, *maskWiFi=NULL, vio;
	double loLeakageDbRealArray [LP_BUFFER_SIZE] = {0.0};
	double loLeakageDbImagArray [LP_BUFFER_SIZE] = {0.0};
	int loLeakageDbArrayLength, bufferSizeX, bufferSizeY;
		
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Segms_Analyze80211b(numSegm: %d, eq_taps: %d, DCremove11b_flag: %d, method_11b: %d)"), m_numSegm, m_IQAnalySetting.wifi_11b_eq_taps, m_IQAnalySetting.wifi_11b_DCremove11b_flag, m_IQAnalySetting.wifi_11b_method_11b);
#endif
	if((errStatus=LP_Segms_Analyze80211b(m_numSegm, m_IQAnalySetting.wifi_11b_eq_taps, m_IQAnalySetting.wifi_11b_DCremove11b_flag,m_IQAnalySetting.wifi_11b_method_11b)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Segms_Analyze80211b failed, errMsg: "));
		goto go_Error;
	}
	
	pWifiTxResult->rmsPower = LP_Segms_GetScalarMeasurement(m_numSegm, "rmsPower", 0);
	if(pWifiTxResult->rmsPower != NA_NUMBER) pWifiTxResult->rmsPower += m_IQVSASettings.Pathloss;
	if(m_IQVSASettings.AnalyzeType == STANDARD_802_11_B_CAL) goto go_End;

	pWifiTxResult->DataRate = LP_Segms_GetScalarMeasurement(m_numSegm, "bitRateInMHz", 0);
	pWifiTxResult->evmAll = LP_Segms_GetScalarMeasurement(m_numSegm, "evmAll", 0);
	pWifiTxResult->freqErr = LP_Segms_GetScalarMeasurement(m_numSegm, "freqErr", 0);	
	pWifiTxResult->clockErr = LP_Segms_GetScalarMeasurement(m_numSegm, "clockErr",0);
	pWifiTxResult->freqErr = LP_Segms_GetScalarMeasurement(m_numSegm, "freqErr",0);

	//Get loLeakage
	loLeakageDbArrayLength = LP_Segms_GetVectorMeasurement(m_numSegm, "loLeakageDb", loLeakageDbRealArray, loLeakageDbImagArray, MAX_BUFFER_SIZE);
	if (loLeakageDbArrayLength)
	{
		pWifiTxResult->loLeakageDb = loLeakageDbRealArray[0];
	}

	//Get Mask
	errStatus = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);
	errStatus = ::LP_Segms_AnalyzeFFT(m_numSegm);

	bufferRealX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferRealY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));

	bufferSizeX = ::LP_Segms_GetVectorMeasurement(m_numSegm, "x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE*8);
	bufferSizeY = ::LP_Segms_GetVectorMeasurement(m_numSegm, "y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE*8);
		
	if((0>=bufferSizeX) || (0>=bufferSizeY)) goto go_End;
	maskWiFi = (double *)malloc(bufferSizeY*sizeof(double));
	testSpectrumMask(bufferRealX, bufferRealY, bufferSizeY, WIFI_11B, maskWiFi, &vio, m_IQVSASettings.Frequency/1000000);
	pWifiTxResult->SpectrumMask = vio;

go_End:
	if (bufferRealX) free(bufferRealX);
	if (bufferImagX) free(bufferImagX);
	if (bufferRealY) free(bufferRealY);
	if (bufferImagY) free(bufferImagY);
	if (maskWiFi) free(maskWiFi);
	return errStatus;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return errStatus;
}

int CLPIQ::Analyze80211ag(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens)
{
	int errStatus = ERR_OK;
	int bufferSizeX, bufferSizeY;	
	double *bufferRealX = NULL, *bufferImagX = NULL, *bufferRealY=NULL, *bufferImagY=NULL, *maskWiFi=NULL, vio;
	int carrierNoUpA, carrierNoUpB, carrierNoLoA, carrierNoLoB;
	int leastMarginCarrier;
	double failMargin, loLeakage;	
	double valueDbUpSectionA, valueDbUpSectionB, valueDbLoSectionA, valueDbLoSectionB;
		
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Segms_Analyze80211ag(m_numSegm: %d, ph_corr_mode: %d, ch_estimate: %d, sym_tim_corr: %d, freq_sync: %d, ampl_track: %d)"), m_numSegm, m_IQAnalySetting.wifi_11ag_ph_corr_mode, m_IQAnalySetting.wifi_11ag_ch_estimate, m_IQAnalySetting.wifi_11ag_sym_tim_corr, m_IQAnalySetting.wifi_11ag_freq_sync, m_IQAnalySetting.wifi_11ag_ampl_track);
#endif
	if((errStatus = LP_Segms_Analyze80211ag(m_numSegm, m_IQAnalySetting.wifi_11ag_ph_corr_mode, m_IQAnalySetting.wifi_11ag_ch_estimate, 
											m_IQAnalySetting.wifi_11ag_sym_tim_corr, m_IQAnalySetting.wifi_11ag_freq_sync,
											m_IQAnalySetting.wifi_11ag_ampl_track)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Segms_Analyze80211ag failed, errMsg: "));
		goto go_Error;
	}

	pWifiTxResult->rmsPower = LP_Segms_GetScalarMeasurement(m_numSegm, "rmsPower", 0);
	if(pWifiTxResult->rmsPower != NA_NUMBER)
	{
		pWifiTxResult->rmsPower += m_IQVSASettings.Pathloss;
	}
	if(m_IQVSASettings.AnalyzeType == STANDARD_802_11_AG_CAL) goto go_End;

	pWifiTxResult->DataRate = LP_Segms_GetScalarMeasurement(m_numSegm, "dataRate",0);
	pWifiTxResult->evmAll = LP_Segms_GetScalarMeasurement(m_numSegm, "evmAll",0);
	pWifiTxResult->codingRate = LP_Segms_GetScalarMeasurement(m_numSegm, "codingRate",0);
	pWifiTxResult->freqErr = LP_Segms_GetScalarMeasurement(m_numSegm, "freqErr",0);
	pWifiTxResult->clockErr = LP_Segms_GetScalarMeasurement(m_numSegm, "clockErr",0);
	pWifiTxResult->phaseErr = LP_Segms_GetScalarMeasurement(m_numSegm, "phaseErr",0);
	pWifiTxResult->loLeakageDb = LP_Segms_GetScalarMeasurement(m_numSegm, "dcLeakageDbc",0);	

	//Get SpectFlatnes
	errStatus = LoResultSpectralFlatness11AG_byAPI(&carrierNoUpA, &carrierNoUpB, &carrierNoLoA, &carrierNoLoB,
													&failMargin, &leastMarginCarrier, &loLeakage,
													&valueDbUpSectionA, &valueDbUpSectionB, &valueDbLoSectionA, &valueDbLoSectionB);	
	pWifiTxResult->SpectFlatness = failMargin;

	//Get Mask
	errStatus = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);
	::LP_Segms_AnalyzeFFT(m_numSegm);

	bufferRealX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferRealY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));

	bufferSizeX = ::LP_Segms_GetVectorMeasurement(m_numSegm, "x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE*8);
	bufferSizeY = ::LP_Segms_GetVectorMeasurement(m_numSegm, "y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE*8);
		
	if((0>=bufferSizeX)||(0>=bufferSizeY)) goto go_End;
	maskWiFi = (double *)malloc(bufferSizeY*sizeof(double));
	testSpectrumMask(bufferRealX, bufferRealY, bufferSizeY, WIFI_11AG, maskWiFi, &vio, m_IQVSASettings.Frequency/1000000);
	pWifiTxResult->SpectrumMask = vio;

go_End:
	if (bufferRealX) free(bufferRealX);
	if (bufferImagX) free(bufferImagX);
	if (bufferRealY) free(bufferRealY);
	if (bufferImagY) free(bufferImagY);
	if (maskWiFi) free(maskWiFi);
	return errStatus;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return errStatus;
}

int CLPIQ::Analyze80211n(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens)
{
	int errStatus = ERR_OK;
	int bufferSizeX, bufferSizeY;
	double *bufferRealX = NULL, *bufferImagX = NULL, *bufferRealY=NULL, *bufferImagY=NULL, *maskWiFi=NULL, vio;
	int carrierNoUpA, carrierNoUpB, carrierNoLoA, carrierNoLoB;
	double failMargin, loLeakage;
	int leastMarginCarrier;
	double valueDbUpSectionA, valueDbUpSectionB, valueDbLoSectionA, valueDbLoSectionB;
		
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Segms_Analyze80211n(numSegm: %d, type: %s, mode: %s, enablePhaseCorr: %d, enableSymTimingCorr: %d, enableAmplitudeTracking: %d, decodePSDU: %d, enableFullPacketChannelEst: %d, packetFormat: %d, frequencyCorr: %d)"), m_numSegm, m_IQAnalySetting.wifi_11n_type, m_IQAnalySetting.wifi_11n_mode, m_IQAnalySetting.wifi_11n_enablePhaseCorr, m_IQAnalySetting.wifi_11n_enableSymTimingCorr, m_IQAnalySetting.wifi_11n_enableAmplitudeTracking, m_IQAnalySetting.wifi_11n_decodePSDU, m_IQAnalySetting.wifi_11n_enableFullPacketChannelEst, m_IQAnalySetting.wifi_11n_packetFormat, m_IQAnalySetting.wifi_11n_frequencyCorr);
#endif
	if((errStatus = LP_Segms_Analyze80211n(m_numSegm, m_IQAnalySetting.wifi_11n_type, m_IQAnalySetting.wifi_11n_mode, m_IQAnalySetting.wifi_11n_enablePhaseCorr,
											m_IQAnalySetting.wifi_11n_enableSymTimingCorr, m_IQAnalySetting.wifi_11n_enableAmplitudeTracking,
											m_IQAnalySetting.wifi_11n_decodePSDU, m_IQAnalySetting.wifi_11n_enableFullPacketChannelEst,
											_T(""), m_IQAnalySetting.wifi_11n_packetFormat, m_IQAnalySetting.wifi_11n_frequencyCorr)) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Segms_Analyze80211n failed, errMsg: "));
		goto go_Error;
	}

	pWifiTxResult->rmsPower = LP_Segms_GetScalarMeasurement(m_numSegm, "rmsPower", 0);
	//printf(">> %f\n", pWifiTxResult->rmsPower);
	if(pWifiTxResult->rmsPower != NA_NUMBER) pWifiTxResult->rmsPower += m_IQVSASettings.Pathloss;
	if(m_IQVSASettings.AnalyzeType == STANDARD_802_11_N_CAL) goto go_End;

	pWifiTxResult->evmAll = LP_Segms_GetScalarMeasurement(m_numSegm, "evmAvgAll",0);
	pWifiTxResult->loLeakageDb = LP_Segms_GetScalarMeasurement(m_numSegm, "dcLeakageDbc",0);
	pWifiTxResult->freqErr = LP_Segms_GetScalarMeasurement(m_numSegm, "freqErrorHz",0);
	pWifiTxResult->clockErr = LP_Segms_GetScalarMeasurement(m_numSegm, "symClockErrorPpm",0);	
	pWifiTxResult->DataRate = LP_Segms_GetScalarMeasurement(m_numSegm, "rateInfo_dataRateMbps",0);

	//Get Flatness 	
	errStatus = LoResultSpectralFlatness11N_byAPI(1,(m_IQVSASettings.BandWidth == BW_20MHZ)?WIFI_11N_HT20:WIFI_11N_HT40,
													&carrierNoUpA, &carrierNoUpB, &carrierNoLoA, &carrierNoLoB,
													&failMargin, &leastMarginCarrier, &loLeakage,
													&valueDbUpSectionA, &valueDbUpSectionB, &valueDbLoSectionA, &valueDbLoSectionB, 0);	
	pWifiTxResult->SpectFlatness = failMargin;

	//Get Mask
	errStatus = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);
	errStatus = ::LP_Segms_AnalyzeFFT(m_numSegm);

	bufferRealX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferRealY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));

	bufferSizeX = ::LP_Segms_GetVectorMeasurement(m_numSegm, "x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE*8);
	bufferSizeY = ::LP_Segms_GetVectorMeasurement(m_numSegm, "y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE*8);	
		
	if((0>=bufferSizeX)||(0>=bufferSizeY)) goto go_End;
	maskWiFi = (double *)malloc(bufferSizeY*sizeof(double));
	testSpectrumMask(bufferRealX, bufferRealY, bufferSizeY, (m_IQVSASettings.BandWidth == BW_20MHZ)?WIFI_11N_HT20:WIFI_11N_HT40, maskWiFi, &vio, m_IQVSASettings.Frequency/1000000);
	pWifiTxResult->SpectrumMask = vio;

	if(m_phSpectrumMask)
	{
		COPYDATASTRUCT CopyData;
		int nBuffSize = sizeof(TCHAR)*STRING_LENS_64 + sizeof(int) + (LP_BUFFER_SIZE*8)*sizeof(double) + bufferSizeY*sizeof(double) + (LP_BUFFER_SIZE*8)*sizeof(double);
		TCHAR *pSendBuffer = new TCHAR[nBuffSize];

		memset(pSendBuffer, 0, nBuffSize);
		memcpy_s(&pSendBuffer[0], nBuffSize, pWifiTxResult->TestItem, sizeof(TCHAR)*STRING_LENS_64);

		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64], nBuffSize, &bufferSizeY, sizeof(int));
		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)], nBuffSize, bufferRealX, (LP_BUFFER_SIZE*8)*sizeof(double));
		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)+(LP_BUFFER_SIZE*8)*sizeof(double)], nBuffSize, maskWiFi, bufferSizeY*sizeof(double));
		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)+(LP_BUFFER_SIZE*8)*sizeof(double)+bufferSizeY*sizeof(double)], nBuffSize, bufferRealY, (LP_BUFFER_SIZE*8)*sizeof(double));
		
		CopyData.dwData = 0;
		CopyData.cbData = nBuffSize;
		CopyData.lpData = pSendBuffer;

		::SendMessage(*m_phSpectrumMask, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&CopyData);

		DEL_ARRAY(pSendBuffer)
	}

go_End:
	if (bufferRealX) free(bufferRealX);
	if (bufferImagX) free(bufferImagX);
	if (bufferRealY) free(bufferRealY);
	if (bufferImagY) free(bufferImagY);
	if(maskWiFi) free(maskWiFi);
	return errStatus;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return errStatus;
}

int CLPIQ::Analyze80211ac(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens)
{
	int errStatus = ERR_OK;
	int bufferSizeX, bufferSizeY;
	double *bufferRealX = NULL, *bufferImagX = NULL, *bufferRealY=NULL, *bufferImagY=NULL, *maskWiFi=NULL, vio;
	int carrierNoUpA, carrierNoUpB, carrierNoLoA, carrierNoLoB;
	double failMargin, loLeakage;
	int leastMarginCarrier, cbw;
	double valueDbUpSectionA, valueDbUpSectionB, valueDbLoSectionA, valueDbLoSectionB;
		
#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_Segms_Analyze80211ac(numSegm: %d, mode: %s, enablePhaseCorr: %d, enableSymTimingCorr: %d, enableAmplitudeTracking: %d, decodePSDU: %d, enableFullPacketChannelEst: %d, frequencyCorr: %d, packetFormat: %d)"), m_numSegm, m_IQAnalySetting.wifi_11ac_mode, m_IQAnalySetting.wifi_11ac_enablePhaseCorr, m_IQAnalySetting.wifi_11ac_enableSymTimingCorr, m_IQAnalySetting.wifi_11ac_enableAmplitudeTracking, m_IQAnalySetting.wifi_11ac_decodePSDU, m_IQAnalySetting.wifi_11ac_enableFullPacketChannelEst, m_IQAnalySetting.wifi_11ac_frequencyCorr, m_IQAnalySetting.wifi_11ac_packetFormat);
	if(strcmp(m_IQAnalySetting.wifi_11ac_mode, _T("composite"))==0) CCommFunc::OutputTracelog(_T("Ref file: %s"), m_IQVSASettings.szRefFile);
#endif

	if((errStatus=LP_Segms_Analyze80211ac(m_numSegm, m_IQAnalySetting.wifi_11ac_mode, m_IQAnalySetting.wifi_11ac_enablePhaseCorr,
			m_IQAnalySetting.wifi_11ac_enableSymTimingCorr, m_IQAnalySetting.wifi_11ac_enableAmplitudeTracking, 
			m_IQAnalySetting.wifi_11ac_decodePSDU, m_IQAnalySetting.wifi_11ac_enableFullPacketChannelEst,
			m_IQAnalySetting.wifi_11ac_frequencyCorr, (strcmp(m_IQAnalySetting.wifi_11ac_mode, _T("composite"))==0)?m_IQVSASettings.szRefFile:NULL, m_IQAnalySetting.wifi_11ac_packetFormat)) != ERR_OK)	
	{
		strcpy_s(szRespMsg, nLens, _T("LP_Segms_Analyze80211ac failed, errMsg: "));
		goto go_Error;
	}	

	if(strcmp(m_IQAnalySetting.wifi_11ac_mode, _T("composite")) == 0)
	{
		for(int n= 0 ; n<4 ; n++)
		{
			pWifiTxResult->CompositePower[n] = LP_Segms_GetScalarMeasurement(m_numSegm, "rxRmsPowerDb", n);
			if(pWifiTxResult->CompositePower[n] != NA_NUMBER) pWifiTxResult->CompositePower[n] += m_IQVSASettings.AllPathloss[n];
		}

		if(m_IQVSASettings.AnalyzeType == STANDARD_802_11_AC_CAL) goto go_End;
	}else 
	{
		if((pWifiTxResult->rmsPower = LP_Segms_GetScalarMeasurement(m_numSegm, "rmsPower", 0)) != NA_NUMBER) pWifiTxResult->rmsPower += m_IQVSASettings.Pathloss;
	}

	pWifiTxResult->evmAll = LP_Segms_GetScalarMeasurement(m_numSegm, "evmAvgAll",0);
	pWifiTxResult->loLeakageDb = LP_Segms_GetScalarMeasurement(m_numSegm, "dcLeakageDbc",0);
	pWifiTxResult->freqErr = LP_Segms_GetScalarMeasurement(m_numSegm, "freqErrorHz",0);
	pWifiTxResult->clockErr = LP_Segms_GetScalarMeasurement(m_numSegm, "symClockErrorPpm",0);	

	//Get
	if(m_IQVSASettings.BandWidth == BW_20MHZ) cbw = BW_20MHZ;
	else if(m_IQVSASettings.BandWidth == BW_40MHZ) cbw = BW_40MHZ;
	else cbw = BW_80MHZ;

	errStatus = LoResultSpectralFlatness11AC_byAPI(1, cbw,
													&carrierNoUpA, &carrierNoUpB, &carrierNoLoA, &carrierNoLoB,
													&failMargin, &leastMarginCarrier, &loLeakage,
													&valueDbUpSectionA, &valueDbUpSectionB, &valueDbLoSectionA, &valueDbLoSectionB,
													0);
	pWifiTxResult->SpectFlatness = failMargin;

	//Get Mask
	errStatus = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);
	errStatus = ::LP_Segms_AnalyzeFFT(m_numSegm);

	bufferRealX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagX = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferRealY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));
	bufferImagY = (double *)malloc((LP_BUFFER_SIZE*8)*sizeof(double));

	bufferSizeX = ::LP_Segms_GetVectorMeasurement(m_numSegm, "x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE*8);
	bufferSizeY = ::LP_Segms_GetVectorMeasurement(m_numSegm, "y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE*8);
		
	if((0>=bufferSizeX)||(0>=bufferSizeY)) goto go_End;
	maskWiFi = (double *)malloc(bufferSizeY*sizeof(double));
	testSpectrumMask(bufferRealX, bufferRealY, bufferSizeY, WIFI_11AC_VHT80, maskWiFi, &vio, m_IQVSASettings.Frequency/1000000);
	pWifiTxResult->SpectrumMask = vio;

	if(m_phSpectrumMask)
	{
		COPYDATASTRUCT CopyData;
		int nBuffSize = sizeof(TCHAR)*STRING_LENS_64 + sizeof(int) + (LP_BUFFER_SIZE*8)*sizeof(double) + bufferSizeY*sizeof(double) + (LP_BUFFER_SIZE*8)*sizeof(double);
		TCHAR *pSendBuffer = new TCHAR[nBuffSize];

		memset(pSendBuffer, 0, nBuffSize);
		memcpy_s(&pSendBuffer[0], nBuffSize, pWifiTxResult->TestItem, sizeof(TCHAR)*STRING_LENS_64);

		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64], nBuffSize, &bufferSizeY, sizeof(int));
		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)], nBuffSize, bufferRealX, (LP_BUFFER_SIZE*8)*sizeof(double));
		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)+(LP_BUFFER_SIZE*8)*sizeof(double)], nBuffSize, maskWiFi, bufferSizeY*sizeof(double));
		memcpy_s(&pSendBuffer[sizeof(TCHAR)*STRING_LENS_64+sizeof(int)+(LP_BUFFER_SIZE*8)*sizeof(double)+bufferSizeY*sizeof(double)], nBuffSize, bufferRealY, (LP_BUFFER_SIZE*8)*sizeof(double));

		CopyData.dwData = 0;
		CopyData.cbData = nBuffSize;
		CopyData.lpData = pSendBuffer;

		::SendMessage(*m_phSpectrumMask, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&CopyData);

		DEL_ARRAY(pSendBuffer)
	}

go_End:
	if (bufferRealX) free(bufferRealX);
	if (bufferImagX) free(bufferImagX);
	if (bufferRealY) free(bufferRealY);
	if (bufferImagY) free(bufferImagY);
	if (maskWiFi) free(maskWiFi);
	return errStatus;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return errStatus;
}

int CLPIQ::AnalyzeZigbee(ZB_RESULT *pZBResult, LPSTR szRespMsg, int nLens)
{
	int errStatus = ERR_OK;
	int bufferSizeX, bufferSizeY;
	double *maskZigbee = NULL, vio;
	double *bufferRealX = NULL, *bufferImagX = NULL, *bufferRealY=NULL, *bufferImagY=NULL;

#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_AnalyzeZigbee()"));
#endif
	if((errStatus = LP_AnalyzeZigbee()) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_AnalyzeZigbee failed, errMsg: "));
		goto go_Error;
	}

	pZBResult->evmAll = LP_GetScalarMeasurement("evmAll",0);
	pZBResult->evmAllOffset = LP_GetScalarMeasurement("evmAllOffset",0);
	pZBResult->rxRmsPowerNoGapDbm = LP_GetScalarMeasurement("rxRmsPowerNoGapDbm",0);
	pZBResult->rxPeakPowerDbm = LP_GetScalarMeasurement("rxPeakPowerDbm",0);
	pZBResult->freqOffsetFineHz = LP_GetScalarMeasurement("freqOffsetFineHz",0);
	pZBResult->symClockErrorPpm = LP_GetScalarMeasurement("symClockErrorPpm",0);
	pZBResult->phaseNoiseDegRmsAll = LP_GetScalarMeasurement("phaseNoiseDegRmsAll",0);

	//Mask
	errStatus = ::LP_SetAnalysisParameterInteger("AnalyzeFFT", "vsaNum", 1);
	errStatus = ::LP_AnalyzeFFT();

	bufferRealX = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
	bufferImagX = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
	bufferRealY = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));
	bufferImagY = (double *)malloc((MAX_BUFFER_SIZE*8)*sizeof(double));

	bufferSizeX = ::LP_GetVectorMeasurement("x", bufferRealX, bufferImagX, MAX_BUFFER_SIZE*8);
	bufferSizeY = ::LP_GetVectorMeasurement("y", bufferRealY, bufferImagY, MAX_BUFFER_SIZE*8);
	if((0>=bufferSizeX)||(0>=bufferSizeY)) goto go_End;

	maskZigbee = (double *)malloc(bufferSizeY*sizeof(double));
	testSpectrumMask(bufferRealX, bufferRealY, bufferSizeY, ZIGBEE, maskZigbee, &vio, m_IQVSASettings.Frequency/1000000);
	pZBResult->SpectrumMask = vio;

go_End:
	if (bufferRealX) free(bufferRealX);
	if (bufferImagX) free(bufferImagX);
	if (bufferRealY) free(bufferRealY);
	if (bufferImagY) free(bufferImagY);
	if (maskZigbee) free(maskZigbee);
	return errStatus;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return errStatus;
}

int CLPIQ::AnalyzeBluetooth(BT_RESULT *pBTResult, LPSTR szRespMsg, int nLens)
{
	int errStatus = ERR_OK;

#if (_DEBUG)
	CCommFunc::OutputTracelog(_T("LP_AnalyzeBluetooth(0, \"AllPlus\")"));
#endif
	if((errStatus = LP_AnalyzeBluetooth(0, "AllPlus")) != ERR_OK)
	{
		strcpy_s(szRespMsg, nLens, _T("LP_AnalyzeBluetooth failed, errMsg: "));
		goto go_Error;
	}

	pBTResult->P_av_each_burst = 10 * log10(LP_GetScalarMeasurement("P_av_each_burst",0));
	pBTResult->P_pk_each_burst = 10 * log10(LP_GetScalarMeasurement("P_pk_each_burst",0));
	
	if((pBTResult->btDataRate = LP_GetScalarMeasurement("dataRateDetect",0)) == 4)
	{
		double le_deltaF2Max[2048] = {0};
		int le_deltaF2Max_len = 2048;

		pBTResult->leFreqOffset = LP_GetScalarMeasurement("leFreqOffset",0);
		pBTResult->leDeltaF1Avg = LP_GetScalarMeasurement("leDeltaF1Avg",0);
		pBTResult->leDeltaF2Max = LP_GetScalarMeasurement("leDeltaF2Max",0);
		pBTResult->leDeltaF2Avg = LP_GetScalarMeasurement("leDeltaF2Avg",0);
		pBTResult->leFreqDevSyncAv = LP_GetScalarMeasurement("leFreqDevSyncAv",0);
		pBTResult->leFnMax = LP_GetScalarMeasurement("leFnMax",0);
		pBTResult->leDeltaF0FnMax = LP_GetScalarMeasurement("leDeltaF0FnMax",0);
		pBTResult->leDeltaF1F0 = LP_GetScalarMeasurement("leDeltaF1F0",0);
		pBTResult->leDeltaFnFn_5Max = LP_GetScalarMeasurement("leDeltaFnFn_5Max",0);
		pBTResult->leIsCrcOk = LP_GetScalarMeasurement("leIsCrcOk",0);
		pBTResult->lePduLength = LP_GetScalarMeasurement("lePduLength",0);
		pBTResult->leCount = LP_GetVectorMeasurement("leDeltaF2Max", le_deltaF2Max, NULL, le_deltaF2Max_len);
	}else
	{
		pBTResult->freq_deviation = LP_GetScalarMeasurement("freq_deviation",0);
		pBTResult->deltaF2AvAccess = LP_GetScalarMeasurement("deltaF2AvAccess",0);
		pBTResult->deltaF2MaxAccess = LP_GetScalarMeasurement("deltaF2MaxAccess",0);
		pBTResult->freq_est = LP_GetScalarMeasurement("freq_est",0);
		pBTResult->freq_estHeader = LP_GetScalarMeasurement("freq_estHeader",0);

		if(pBTResult->btDataRate == 1)
		{
			pBTResult->freq_drift = LP_GetScalarMeasurement("freq_drift",0);
			pBTResult->deltaF1Average = LP_GetScalarMeasurement("deltaF1Average",0);
			pBTResult->deltaF2Average = LP_GetScalarMeasurement("deltaF2Average",0);
			pBTResult->deltaF2Max = LP_GetScalarMeasurement("deltaF2Max",0);
			pBTResult->maxfreqDriftRate = LP_GetScalarMeasurement("maxfreqDriftRate",0);
		}else
		{
			pBTResult->EdrEVMAv = LP_GetScalarMeasurement("EdrEVMAv",0);
			pBTResult->EdrEVMpk = LP_GetScalarMeasurement("EdrEVMpk",0);
			pBTResult->EdrEVMvsTime = LP_GetScalarMeasurement("EdrEVMvsTime",0);
			pBTResult->EdrPowDiffdB = LP_GetScalarMeasurement("EdrPowDiffdB",0);
			pBTResult->EdrOmegaI = LP_GetScalarMeasurement("EdrOmegaI",0);
			pBTResult->EdrExtremeOmega0 = LP_GetScalarMeasurement("EdrExtremeOmega0",0);
			pBTResult->EdrExtremeOmegaI0 = LP_GetScalarMeasurement("EdrExtremeOmegaI0",0);
		}
	}

	return errStatus;
go_Error:
	if(errStatus != ERR_OK) strcat_s(szRespMsg, nLens, LP_GetErrorString(errStatus));
	return errStatus;
}