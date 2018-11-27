#pragma once

#include "..\..\..\Lib\rfDefine.h"
#include "..\..\..\Lib\vDUTexport.h"

#include "..\_Utility\mySocket\mySocket.h"
#include "..\_Utility\Parser\Parser.h"
#include "..\_Utility\Logger\Logger.h"
#include "..\_Utility\Converter\Converter.h"
#include "..\_Utility\myPing\myPing.h"

#include "..\_EquipController\EquipCtrlBase.h"
#include "..\_EquipController\LitePoint\LPIQ.h"
#include "..\_EquipController\NationalInstruments\NI_GIT.h"

class CTestItems
{
public:
	CTestItems(void);
	~CTestItems(void);

public:
	/*CALLBACK*/
	static bool CALLBACK openTelnet(TCHAR* szIPAddr, TCHAR* szLoginPrompt, TCHAR* szLoginID, TCHAR* szPwdPrompt, TCHAR* szLoginPwd, TCHAR *szShellPrompt, LPSTR szRespMsg, int nLens);
	static void CALLBACK closeTelnet();
	static bool CALLBACK sendTelnetCmd(TCHAR* szCmd, TCHAR* szChkString, int nTimeout, LPSTR szRespMsg, int nLens, bool bChkResponse);

	static bool CALLBACK getTesterLock(LPSTR szRespMsg, int nLens);
	static bool CALLBACK releaseTesterLock(LPSTR szRespMsg, int nLens);

	static bool CALLBACK setupVSA(RF_SETTINGS *pRFSettings, LPSTR szRespMsg, int nLens);
	static bool CALLBACK setupVSG(RF_SETTINGS *pRFSettings, LPSTR szModFile, int frameCnt);
	static bool CALLBACK startMeasure(MEASURE_RESULT *pMeasureResult, LPSTR szRespMsg, int nLens);
	static bool CALLBACK startGenerate(int frameCnt, LPSTR szRespMsg, int nLens);

	static void CALLBACK outputLog(LOGTYPE logType, TCHAR *szFormat, ...);

	//Initialize & Terminate
	void InitializeTestItem(DUT_SETTINGS *pDutSettings, TEST_RESULT *pTestResult, CParser *pParser, CLogger *pLogger, bool bCsvExport=false);
	void TerminateTestItem();

	RUNSTATUS ARC_FORTEST(CString strParameter);

	RUNSTATUS ARC_PING_TO_ALIVE(CString strParameter);
	RUNSTATUS ARC_PING_TO_DEAD(CString strParameter);

	//DEVICE
	RUNSTATUS ATC_CONNECT_TESTER(CString strParameter);
	RUNSTATUS ATC_TESTER_RELEASE_CONTROL();
	RUNSTATUS LOAD_IQ_PARAMETER(CString strParameter);
	RUNSTATUS SETUP_SPECTRUM_MASK(CString strParameter);

	//PATHLOSS
	RUNSTATUS LOAD_PATHLOSS(CString strParameter);

	//VDUT
	RUNSTATUS ATC_INSERT_DUT(CString strParameter);
	RUNSTATUS ATC_REBOOT_DUT(CString strParameter);
	RUNSTATUS ATC_REMOVE_DUT();

	//CALIBRATION
	RUNSTATUS WIFI_11AC_TX_CALIBRATION(CString strParameter, CString strSPEC, CString strOptions);
	RUNSTATUS WIFI_11AC_RX_CALIBRATION(CString strParameter, CString strSPEC, CString strOptions);

	//VERIFICATION
	RUNSTATUS WIFI_11AC_TX_VERIFY_ALL(CString strParameter, CString strSPEC, CString strOptions);
	RUNSTATUS WIFI_11AC_RX_VERIFY_PER(CString strParameter, CString strSPEC, CString strOptions);
	RUNSTATUS WIFI_11AC_RX_SWEEP(CString strParameter, CString strSPEC, CString strOptions);

	/*CSV*/
	bool					m_CsvExport;
	CStringArray			m_csv2gTxTitle, m_csv5gTxTitle, m_csv2gRxTitle, m_csv5gRxTitle;
	CStringArray			m_csv2gTxPower, m_csv5gTxPower, m_csv2gEVM, m_csv5gEVM, m_csv2gPER, m_csv5gPER;

private:

	void SetErrorReport(LPSTR lpErrorCode, LPSTR lpTestItem, TCHAR *szFormat, ...);
	void ShowIQParameter(IQANALYZE_SETTING *pIQAnalySetting); 

	bool Init_IQDevice(CString strParameter);
	bool Init_NIDevice(CString strParameter);
	bool checkEquipCtrl(CString strTestItem);
	
	bool wifiCompareTxResult(CString strTestItem, WIFI_TXRESULT *pWifiTxResult, SPEC_SETTINGS *pSPECSettings, int MeasureCnt);
	bool wifiCompareRxResult(CString strTestItem, WIFI_RXRESULT *pWifiRxResult, SPEC_SETTINGS *pSPECSettings, int MeasureCnt);


	//Class	
	CParser					*m_pParser;
	static CLogger			*m_pLogger;
	static mySocket			m_Socket;
	myPing					m_Ping;
	static EquipCtrlbase		*m_EquipCtrl;

	//Struct
	DUT_SETTINGS				*m_pDutSettings;
	TEST_RESULT				*m_pTestResult;

	static RF_TYPE			m_rfMeasurer;
	static TCHAR				m_szRespMsg[STRING_LENS_4096];

	HWND						m_hSpectrumMask;
};

