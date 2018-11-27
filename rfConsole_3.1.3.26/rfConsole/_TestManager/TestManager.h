#pragma once

#include "..\..\..\Lib\rfDefine.h"

#include "..\_TestItems\TestItems.h"

#include "..\_Utility\CommFunc\CommFunc.h"
#include "..\_Utility\Logger\Logger.h"
#include "..\_Utility\Parser\Parser.h"
#include "..\_Utility\myMD5\myMD5.h"


class CTestManager
{
public:
	CTestManager(void);
	~CTestManager(void);

public:
	void Initialize(HANDLE hRunFinish, LPSTR lpTestFlow, LPSTR lpMAC, LPSTR lpSN, bool bExit);
	void Terminate();

	/*Thread*/
	bool StartRun();
	static UINT FlowCtrlThread(LPVOID pParam);
	

private:	
	void ResetTest();
	void ShowInformation();
	void DisplayResult();
	bool md5Verification(LPSTR lpFile, LPSTR lpMD5);

	RUNSTATUS RunActionItems(TEST_ITEM *pTItem);

	HANDLE				m_hThreadFinish, m_hCloseProgram;
	bool					m_ThreadRunning;
	bool					m_bExit;

	//Class
	myMD5				m_MD5;
	CParser				m_Parser;
	CLogger				m_Logger;
	CTestItems			m_TestItem;


	//Struct
	TEST_SETTINGS		m_TestSettings;
	DUT_SETTINGS			m_DutSettings;

	TESTITEMARY			m_TestItemAry;
	TEST_RESULT			m_TestResult;

	int					m_PassRun, m_RunCnt;
	LPSTR				m_lpTestFlow;

	
};

