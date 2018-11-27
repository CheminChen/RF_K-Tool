#include "StdAfx.h"
#include "TestManager.h"

#define MAX_STACK_SIZE 65535*100

CTestManager::CTestManager(void)
{
	m_ThreadRunning = true;
	m_PassRun = m_RunCnt = 0;
	memset(&m_TestSettings, 0, sizeof(TEST_SETTINGS));
	memset(&m_DutSettings, 0, sizeof(DUT_SETTINGS));
	memset(&m_TestResult, 0, sizeof(TEST_RESULT));
}

CTestManager::~CTestManager(void)
{
}

void CTestManager::Initialize(HANDLE hCloseProgram, LPSTR lpTestFlow, LPSTR lpMAC, LPSTR lpSN, bool bExit)
{
	TCHAR *pszTmp = new TCHAR[MAX_PATH];

	CCommFunc::GetLocalPath();
	m_hCloseProgram = hCloseProgram;
	m_lpTestFlow = lpTestFlow;
	
	m_bExit = bExit;
	//Parser
	m_Parser.InitializeParser(lpTestFlow, &m_Logger);
	m_Parser.ParserTestSettings(&m_TestSettings);
	m_Parser.ParserDUTSettings(&m_DutSettings);
	if(strlen(lpMAC)) strcpy_s(m_DutSettings.MAC, sizeof(m_DutSettings.MAC), lpMAC);
	if(strlen(lpSN)) strcpy_s(m_DutSettings.SN, sizeof(m_DutSettings.SN), lpSN);

	//Logger
	memset(pszTmp, 0, MAX_PATH);
	sprintf_s(pszTmp, MAX_PATH, _T("%sTracelog.log"), CCommFunc::m_LogPath);
	m_Logger.StartLogger(m_TestSettings.EnableDebugMsg, pszTmp);

	//MD
	m_MD5.m_pLogger = &m_Logger;

	//TestItem
	m_TestItem.InitializeTestItem(&m_DutSettings, &m_TestResult, &m_Parser, &m_Logger, m_TestSettings.ExportCSV);
	
	DEL_ARRAY(pszTmp)
}

void CTestManager::Terminate()
{
	m_ThreadRunning = false;
	CLEAN_CARRAY(m_TestItemAry);
	WaitForSingleObject(m_hThreadFinish, 1000);

	m_Parser.TerminateParser();
	m_Logger.StopLogger();
	m_TestItem.TerminateTestItem();

	SetEvent(m_hCloseProgram);	//For Close console window (exe)
}

void CTestManager::ResetTest()
{
	CTime cTime = CTime::GetCurrentTime();
	strcpy_s(m_DutSettings.Timestamp, sizeof(m_DutSettings.Timestamp), cTime.Format(_T("%m-%d_%H-%M-%S")).GetBuffer());

	memset(&m_TestResult, 0, sizeof(m_TestResult));
	m_TestResult.TestResult = true;
	m_TestResult.TotalRun = (m_TestSettings.LoopTest > 1)?m_TestSettings.LoopTest:1;
	if(!((m_TestSettings.LoopTest == 1) || (m_TestSettings.LoopTest == 0))) m_TestResult.PassedRun = m_PassRun;
}

void CTestManager::ShowInformation()
{
	TCHAR *pInfo = new TCHAR[STRING_LENS_2048], *pMD5 = new TCHAR[MD5_LENGTH];
	bool bMD5check = false;
	memset(pInfo, 0, STRING_LENS_2048);
	memset(pMD5, 0, MD5_LENGTH);

	bMD5check = md5Verification(m_lpTestFlow, pMD5);
	sprintf_s(pInfo, STRING_LENS_2048, _T("\r\n\r\n/****************************************************************************/\
										   \r\n/* ARCADAYN RFCP 2018\
										   \r\n/* %-20s : v%s/ v%s\
										   \r\n/* %-20s : v%s\
										   \r\n/*\
										   \r\n/* %-20s : %s%s%s\
										   \r\n/* %-20s : %s\
										   \r\n/* %-20s : %s (%s) \
										   \r\n/****************************************************************************/"), 
										   _T("RF Calibration Tool"), CCommFunc::m_ModuleVersion, CCommFunc::m_vDUTversion,
										   _T("IQ Measure Library"), CCommFunc::m_IQMeasureVersion,
										   _T("MAC/ SN"), m_DutSettings.MAC, (strlen(m_DutSettings.SN))?_T("/ "):_T(""), m_DutSettings.SN,
										   _T("Timestamp"), m_DutSettings.Timestamp,
										   _T("MD5"), (strcmp(m_TestSettings.actualMD5, _T(""))==0)?_T("\"\""):m_TestSettings.actualMD5, bMD5check?_T("pass"):_T("fail")
										   );
	m_Logger.OutputLog(TYPE_SUMMARY, COLOR_MAGENTA, pInfo);
	DEL_ARRAY(pInfo)
	DEL_ARRAY(pMD5)
}

bool CTestManager::md5Verification(LPSTR lpFile, LPSTR lpMD5)
{
	bool bRtn = false;
	myMD5 *pMD5 = new myMD5();
	//TCHAR *pExpectedMD5 = new TCHAR[STRING_LENS_256];
		
	/*Reset MD5*/
	if(!(bRtn=WritePrivateProfileString(_T("TEST_SETTINGS"), _T("MD5"), _T(""), lpFile)))
	{
		m_Logger.OutputTracelog(_T("Fail to reset MD5"));
		goto go_End;
	}

	/*Calculate actual MD5*/	
	if(!(bRtn = pMD5->calculateMD5(lpFile, lpMD5, MD5_LENGTH)))
	{
		m_Logger.OutputTracelog(_T("Fail to calculate MD5"));
		goto go_End;
	}

	/*Restore MD5*/
	if(!(bRtn=WritePrivateProfileString(_T("TEST_SETTINGS"), _T("MD5"), m_TestSettings.actualMD5, lpFile)))
	{
		m_Logger.OutputTracelog(_T("Fail to reset MD5"));
		goto go_End;
	}
	printf("> Calculate MD5: %s\n", lpMD5);

	bRtn = (strcmp(lpMD5, m_TestSettings.actualMD5) == 0);
go_End:
	if(pMD5) delete pMD5;
	//DEL_ARRAY(pExpectedMD5)
	return bRtn;
}

void CTestManager::DisplayResult()
{
	TCHAR *pResult = new TCHAR[STRING_LENS_2048];
	memset(pResult, 0, STRING_LENS_2048);

	m_RunCnt++;
	sprintf_s(pResult, STRING_LENS_2048, _T("\r\n\r\n>> [SUMMARY]\r\n   Test Result : %s\r\n   Error Code  : %s\r\n   Fail Item   : %s\r\n   Fail Details  : %s\r\n   Pass Runs   : %-3d/ (%d/%d)\r\n   TestTime    : %3.2f\r\n"),
			m_TestResult.TestResult?_T("Pass"):_T("Fail"), m_TestResult.ErrorCode, m_TestResult.ErrorItem, m_TestResult.ErrorMessage, m_TestResult.PassedRun, m_RunCnt, m_TestResult.TotalRun, m_TestResult.TestTime);
	
	m_Logger.OutputLog(TYPE_SUMMARY, COLOR_MAGENTA, pResult);
	DEL_ARRAY(pResult)
}


bool CTestManager::StartRun()
{
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	m_hThreadFinish = CreateEvent(NULL, TRUE, FALSE, NULL);
	return (AfxBeginThread(FlowCtrlThread, this, THREAD_PRIORITY_NORMAL, MAX_STACK_SIZE, 0, &sa) != NULL);
}

/*INT CTestManager::MaskThread(LPVOID pParam)
{
	CTestManager *pMain = (CTestManager*)pParam;

	pMain->m_pMaskDlg = new CMaskDlg;
	pMain->m_pMaskDlg->Create(IDD_MASKDLG);
	pMain->m_pMaskDlg->ShowWindow(SW_SHOW);
	pMain->m_MaskWnd = pMain->m_pMaskDlg->m_hWnd;
		
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, _T("CloseModelessDialog"));

	MSG msg;
	while(WaitForSingleObject(hEvent, 0) != WAIT_OBJECT_0)
	{
		while(::GetMessage(&msg, NULL, 0, 0))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	CloseHandle(hEvent);
	return 0;
}*/

const int ESCAPE=27;
const int RETURN=13;
UINT CTestManager::FlowCtrlThread(LPVOID pParam)
{
	CTestManager *pMain = (CTestManager*)pParam;	
	CStringArray TestFlowAry;
	TCHAR cInput, *pAutoRunFile = new TCHAR[MAX_PATH];
	int LoopCnt = pMain->m_TestSettings.LoopTest;
	bool bLoopTest = !((pMain->m_TestSettings.LoopTest == 1) || (pMain->m_TestSettings.LoopTest == 0)), bAutoRunTrigger;
	RUNSTATUS RunStatus = STATUS_UNDEFINE;
	CString strCSVlog, strMsg;

	pMain->m_Parser.ParserTestItem(&TestFlowAry, &pMain->m_TestItemAry);
	pMain->m_ThreadRunning = true;

	/*Disable the AutoRun trigger when the looptest is enable!! */
	bAutoRunTrigger = (strcmp(pMain->m_TestSettings.AutoRunTrigger, _T("DISABLE")) != 0) && !bLoopTest;			

	do 
	{
		if(bAutoRunTrigger)
		{
			memset(pAutoRunFile, 0, MAX_PATH);
			sprintf_s(pAutoRunFile, MAX_PATH, _T("%s\\%s"), CCommFunc::m_ModulePath, pMain->m_TestSettings.AutoRunTrigger);

			pMain->m_Logger.OutputLog(TYPE_TITLE, COLOR_WHITE, _T("\r\n>> Wait for AutoRun trigger, File: %s\r\n"), pMain->m_TestSettings.AutoRunTrigger);
			while(!CCommFunc::FileExist(pAutoRunFile)) Sleep(200);
		}

		double dbStartTime = GetTickCount();
		pMain->ResetTest();
		pMain->ShowInformation();

		for(int nTCase=0 ; nTCase<TestFlowAry.GetSize() ; nTCase++)
		{
			CString strTestCase = TestFlowAry.GetAt(nTCase);

			for(int nTItem=0 ; nTItem<pMain->m_TestItemAry.GetSize() ; nTItem++)
			{
				TEST_ITEM *pTItem = pMain->m_TestItemAry.GetAt(nTItem);
				
				if(strTestCase.CompareNoCase(pTItem->TestCase) == 0)
				{
					pMain->m_Logger.OutputLog(TYPE_TITLE, COLOR_YELLOW, _T("\r\n\r\n%-2d. %s"), pTItem->TestIdx, pTItem->TestItem);

					double dbStartTestItem = GetTickCount();
					if(pTItem->RunMode == MODE_SKIP) goto go_Next;
					else if(pTItem->RunMode == MODE_FIXED) RunStatus = pMain->RunActionItems(pTItem);
					else if((pTItem->RunMode == MODE_RUN) || (pTItem->RunMode == MODE_UNDEFINE))
					{
						if((!pMain->m_TestResult.TestResult) && (pMain->m_TestSettings.StopOnFail))
						{
							RunStatus = STATUS_SKIP;
							goto go_Next;
						}else RunStatus = pMain->RunActionItems(pTItem);						
					}					
					
go_Next:
					pMain->m_TestResult.TestResult &= (RunStatus == STATUS_PASS);
					pTItem->TestTime = (float)((GetTickCount() - dbStartTestItem)/1000);

					//Interval sleep
					if((RunStatus == STATUS_PASS) && (pTItem->IntervalSleep != 0))
					{
						strMsg.Format(_T(", Interval Sleep: %dsec"), pTItem->IntervalSleep);
						Sleep(pTItem->IntervalSleep*1000);
					}

					//Display Test Item result
					if(RunStatus == STATUS_PASS) pMain->m_Logger.OutputLog(TYPE_NORMAL, COLOR_GREEN, _T("\r\nPass/ Test time: %2.2fsec%s"), pTItem->TestTime, strMsg);
					else if(RunStatus == STATUS_SKIP) pMain->m_Logger.OutputLog(TYPE_NORMAL, COLOR_YELLOW, _T("\r\nSkip/ Test time: %2.2fsec"), pTItem->TestTime);
					else pMain->m_Logger.OutputLog(TYPE_NORMAL, COLOR_RED, _T("\r\nFail/ Test time: %2.2fsec"), pTItem->TestTime);
				}
			}
		}

		/*Testing completed*/
		pMain->m_TestResult.TestTime = (float)((GetTickCount() - dbStartTime)/1000);
		if(pMain->m_TestResult.TestResult) pMain->m_PassRun = ++pMain->m_TestResult.PassedRun;
		pMain->DisplayResult();

		/*Output CSV*/
		if(pMain->m_TestSettings.ExportCSV)
		{
			pMain->m_Logger.OutputCsv(_T("2.4G-PWR"), pMain->m_DutSettings.Timestamp, pMain->m_DutSettings.MAC, pMain->m_DutSettings.SN, &pMain->m_TestItem.m_csv2gTxTitle, &pMain->m_TestItem.m_csv2gTxPower);
			pMain->m_Logger.OutputCsv(_T("2.4G-EVM"), pMain->m_DutSettings.Timestamp, pMain->m_DutSettings.MAC, pMain->m_DutSettings.SN, &pMain->m_TestItem.m_csv2gTxTitle, &pMain->m_TestItem.m_csv2gEVM);
			pMain->m_Logger.OutputCsv(_T("2.4G-PER"), pMain->m_DutSettings.Timestamp, pMain->m_DutSettings.MAC, pMain->m_DutSettings.SN, &pMain->m_TestItem.m_csv2gRxTitle, &pMain->m_TestItem.m_csv2gPER);

			pMain->m_Logger.OutputCsv(_T("5G-PWR"), pMain->m_DutSettings.Timestamp, pMain->m_DutSettings.MAC, pMain->m_DutSettings.SN, &pMain->m_TestItem.m_csv5gTxTitle, &pMain->m_TestItem.m_csv5gTxPower);
			pMain->m_Logger.OutputCsv(_T("5G-EVM"), pMain->m_DutSettings.Timestamp, pMain->m_DutSettings.MAC, pMain->m_DutSettings.SN, &pMain->m_TestItem.m_csv5gTxTitle, &pMain->m_TestItem.m_csv5gEVM);
			pMain->m_Logger.OutputCsv(_T("5G-PER"), pMain->m_DutSettings.Timestamp, pMain->m_DutSettings.MAC, pMain->m_DutSettings.SN, &pMain->m_TestItem.m_csv5gRxTitle, &pMain->m_TestItem.m_csv5gPER);

			strCSVlog.Format(_T("%s,%s,%s,%4.2f"), pMain->m_DutSettings.Timestamp, pMain->m_DutSettings.MAC, pMain->m_DutSettings.SN, pMain->m_TestResult.TestTime);
			pMain->m_Logger.OutputCsv(_T("TestTime"), strCSVlog);
		}

		/*Check the Loop test*/ 
		if(bLoopTest && (--LoopCnt > 0))
		{
			Sleep(LOOP_INTERVAL_DELAY);
			continue;
		}else
		{
			/*Stop the logger for this time*/
			pMain->m_Logger.StopLogger();

			/*Check AutoRun trigger*/
			if(!pMain->m_bExit)
			{
				if(bAutoRunTrigger)
				{
					if(CCommFunc::FileExist(pAutoRunFile))
					{
						if(!DeleteFile(pAutoRunFile)) 
						{
							pMain->m_Logger.OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("Fail to remove the AutoRun trigger, File: %s, errCode: %d"), pAutoRunFile, GetLastError());
							goto go_End;			
						}
					}
				}else
				{
					printf((bLoopTest)?_T("\nPlease press any key to exit\n"):_T("\nPlease remove DUT, press any key to continue (or Esc to exit)....\n"));
					cInput = getch();
					if(bLoopTest || (!bLoopTest && (cInput == ESCAPE))) break;
				}
			}else pMain->m_ThreadRunning = false;
			

			/*Re-start logger for new one*/
			pMain->m_Logger.StartLogger();
		}
		
	} while (pMain->m_ThreadRunning);

go_End:
	DEL_ARRAY(pAutoRunFile)
	SetEvent(pMain->m_hThreadFinish);
	pMain->Terminate();	
	return 0;
}

RUNSTATUS CTestManager::RunActionItems(TEST_ITEM *pTItem)
{
	RUNSTATUS RunStatus = STATUS_UNDEFINE;	

	if(strcmp(pTItem->TestItem, _T("ARC_FORTEST")) == 0) 
	{
		RunStatus = m_TestItem.ARC_FORTEST(pTItem->TestParameter);
	}
	else if(strcmp(pTItem->TestItem, _T("ARC_PING_TO_ALIVE")) == 0) 
	{
		RunStatus = m_TestItem.ARC_PING_TO_ALIVE(pTItem->TestParameter);
	}
	else if(strcmp(pTItem->TestItem, _T("ARC_PING_TO_DEAD")) == 0) 
	{
		RunStatus = m_TestItem.ARC_PING_TO_DEAD(pTItem->TestParameter);
	}

	else if(strcmp(pTItem->TestItem, _T("SETUP_SPECTRUM_MASK")) == 0) 
	{
		RunStatus = m_TestItem.SETUP_SPECTRUM_MASK(pTItem->TestParameter);
	}

	//DEVICE
	else if(strcmp(pTItem->TestItem, _T("ATC_CONNECT_TESTER")) == 0) 
	{
		RunStatus = m_TestItem.ATC_CONNECT_TESTER(pTItem->TestParameter);
	}
	else if(strcmp(pTItem->TestItem, _T("ATC_TESTER_RELEASE_CONTROL")) == 0) 
	{
		RunStatus = m_TestItem.ATC_TESTER_RELEASE_CONTROL();
	}
	else if(strcmp(pTItem->TestItem, _T("LOAD_IQ_PARAMETER")) == 0) 
	{
		RunStatus = m_TestItem.LOAD_IQ_PARAMETER(pTItem->TestParameter);
	}

	//PATHLOSS
	else if(strcmp(pTItem->TestItem, _T("LOAD_PATHLOSS")) == 0) 
	{
		RunStatus = m_TestItem.LOAD_PATHLOSS(pTItem->TestParameter);
	}

	//DUT
	else if(strcmp(pTItem->TestItem, _T("ATC_INSERT_DUT")) == 0) 
	{
		RunStatus = m_TestItem.ATC_INSERT_DUT(pTItem->TestParameter);
	}
	else if(strcmp(pTItem->TestItem, _T("ATC_REMOVE_DUT")) == 0) 
	{
		RunStatus = m_TestItem.ATC_REMOVE_DUT();
	}
	else if(strcmp(pTItem->TestItem, _T("ATC_REBOOT_DUT")) == 0) 
	{
		RunStatus = m_TestItem.ATC_REBOOT_DUT(pTItem->TestParameter);
	}

	//CALIBRATION
	else if(strcmp(pTItem->TestItem, _T("WIFI_11AC_TX_CALIBRATION")) == 0) 
	{
		RunStatus = m_TestItem.WIFI_11AC_TX_CALIBRATION(pTItem->TestParameter, pTItem->TestSPEC, pTItem->TestOption);
	}
	else if(strcmp(pTItem->TestItem, _T("WIFI_11AC_RX_CALIBRATION")) == 0) 
	{
		RunStatus = m_TestItem.WIFI_11AC_RX_CALIBRATION(pTItem->TestParameter, pTItem->TestSPEC, pTItem->TestOption);
	}	

	//VERIFICATION
	else if(strcmp(pTItem->TestItem, _T("WIFI_11AC_TX_VERIFY_ALL")) == 0) 
	{
		RunStatus = m_TestItem.WIFI_11AC_TX_VERIFY_ALL(pTItem->TestParameter, pTItem->TestSPEC, pTItem->TestOption);
	}
	else if(strcmp(pTItem->TestItem, _T("WIFI_11AC_RX_VERIFY_PER")) == 0) 
	{
		RunStatus = m_TestItem.WIFI_11AC_RX_VERIFY_PER(pTItem->TestParameter, pTItem->TestSPEC, pTItem->TestOption);
	}
	else if(strcmp(pTItem->TestItem, _T("WIFI_11AC_RX_SWEEP")) == 0) 
	{
		RunStatus = m_TestItem.WIFI_11AC_RX_SWEEP(pTItem->TestParameter, pTItem->TestSPEC, pTItem->TestOption);
	}

	return RunStatus;
}