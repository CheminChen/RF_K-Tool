#include "StdAfx.h"
#include "Parser.h"

CParser::CParser(void)
{
}

CParser::~CParser(void)
{
	TerminateParser();
}

void CParser::InitializeParser(LPSTR lpszFile, CLogger *pLogger)
{
	if(pLogger) m_pLogger = pLogger;
	if(lpszFile) m_strTFFile.Format(_T("%s"), lpszFile);
}

void CParser::TerminateParser()
{
	CLEAN_CARRAY(m_PathlossAry)
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

bool CParser::ParserTestItem(CStringArray *pTestFlowAry,TESTITEMARY *pTestItemAry)
{
	TCHAR *pBuffer = new TCHAR[MAX_ARCBUFFER_SIZE], *pszLine = NULL;
	CString strTestCase;
	int nCnt = 0;
	
	/*Parser TestCase*/
	memset(pBuffer, 0, MAX_ARCBUFFER_SIZE);
	if(GetPrivateProfileSection(_T("TEST_FLOW"), pBuffer, MAX_ARCBUFFER_SIZE, m_strTFFile.GetBuffer()) != 0)
	{
		pszLine = pBuffer;

		while(pszLine[0] != '\0')
		{
			if((pszLine[0] != '/') && (pszLine[0] != '/'))
				pTestFlowAry->Add(pszLine);

			int dwLen = (int)strlen(pszLine);
			pszLine += dwLen+1;
		}
	}
	if(pTestFlowAry->GetSize() == 0)
	{
		CCommFunc::OutputTracelog(_T("Fail to parser the session of 'TEST_FLOW'"));
		goto go_End;
	}

	/*Parser TestItem*/
	for(int n=0 ; n<pTestFlowAry->GetSize() ; n++)
	{
		memset(pBuffer, 0, MAX_ARCBUFFER_SIZE);
		strTestCase = pTestFlowAry->GetAt(n);
		if(GetPrivateProfileSection(strTestCase.GetBuffer(), pBuffer, MAX_ARCBUFFER_SIZE, m_strTFFile.GetBuffer()) != 0)
		{
			pszLine = pBuffer;

			while(pszLine[0] != '\0')
			{
				if((pszLine[0] != '/') && (pszLine[0] != '/'))
				{
					TEST_ITEM *pItems = new TEST_ITEM;

					pItems->TestIdx = ++nCnt;
					strcpy_s(pItems->TestCase, STRING_LENS_64, strTestCase.GetBuffer());
					ParserTestParameter(pItems, pszLine);
					pTestItemAry->Add(pItems);
				}

				pszLine += ((int)strlen(pszLine)+1);
			}
		}
	}

go_End:
	DEL_ARRAY(pBuffer)
	return (pTestItemAry->GetSize() > 0);
}

void CParser::ParserTestParameter(TEST_ITEM *pItem, CString strLine)
{
	CString strTestItem, strParameter, strSPEC, strOption, strRunMode, strSleep;
	int nSIdx = -1, nEIdx = -1;

	//Get Test Item
	nSIdx = strLine.Find(_T("("));
	if(nSIdx != -1)
	{
		strTestItem = strLine.Mid(0, nSIdx);
		if(strTestItem.CompareNoCase(_T("")) != 0) strcpy_s(pItem->TestItem, STRING_LENS_64, strTestItem);
		strLine = strLine.Mid(nSIdx+1);
	}	

	//Get Parameter
	nEIdx = strLine.Find(_T(")"));
	if(nEIdx != -1)
	{
		strParameter = strLine.Mid(0, nEIdx);
		if(strParameter.CompareNoCase(_T("")) != 0) strcpy_s(pItem->TestParameter, STRING_LENS_2048, strParameter);
		strLine = strLine.Mid(nEIdx+1);
	}

	//Get SPEC
	nSIdx = strLine.Find(_T(".SPEC("));
	if(nSIdx != -1)
	{
		nSIdx += strlen(_T(".SPEC("));
		nEIdx = strLine.Find(_T(")"));
		strSPEC = strLine.Mid(nSIdx, nEIdx-nSIdx);
		if(strSPEC.CompareNoCase(_T("")) != 0) strcpy_s(pItem->TestSPEC, STRING_LENS_256, strSPEC);
		strLine = strLine.Mid(nEIdx+1);
	}

	//Get Option
	nSIdx = strLine.Find(_T(".OPTION("));
	if(nSIdx != -1)
	{
		nSIdx += strlen(_T(".OPTION("));
		nEIdx = strLine.Find(_T(")"));
		strOption = strLine.Mid(nSIdx, nEIdx-nSIdx);
		if(strOption.CompareNoCase(_T("")) != 0) strcpy_s(pItem->TestOption, STRING_LENS_256, strOption);
		strLine = strLine.Mid(nEIdx+1);
	}

	//Get RunMode
	nSIdx = strLine.Find(_T(".RUNMODE("));
	if(nSIdx != -1)
	{
		nSIdx += strlen(_T(".RUNMODE("));
		nEIdx = strLine.Find(_T(")"));
		strRunMode = strLine.Mid(nSIdx, nEIdx-nSIdx);

		if(strRunMode.CompareNoCase(_T("ALWAYS_RUN")) == 0) pItem->RunMode = MODE_FIXED;
		else pItem->RunMode = MODE_UNDEFINE;

		strLine = strLine.Mid(nEIdx);
	}

	//Get Interval Sleep
	nSIdx = strLine.Find(_T("#"));
	if(nSIdx != -1)
	{
		nSIdx += strlen(_T("#"));
		strSleep = strLine.Mid(nSIdx);
		pItem->IntervalSleep = atoi(strSleep);
	}
}

bool CParser::ParserTestSettings(TEST_SETTINGS *pTestSettings)
{
	bool bRtn = true;

	bRtn &= GetStringfromINIfile(_T("TEST_SETTINGS"), _T("MD5"), _T(""), pTestSettings->actualMD5, MD5_LENGTH);
	bRtn &= GetBoolfromINIfile(_T("TEST_SETTINGS"), _T("ENABLE_DEBUG_MSG"), true, &pTestSettings->EnableDebugMsg);
	bRtn &= GetBoolfromINIfile(_T("TEST_SETTINGS"), _T("STOP_ON_FAILURE"), true, &pTestSettings->StopOnFail);
	bRtn &= GetBoolfromINIfile(_T("TEST_SETTINGS"), _T("EXPORT_CSV_FILE"), false, &pTestSettings->ExportCSV);
	bRtn &= GetBoolfromINIfile(_T("TEST_SETTINGS"), _T("SHOW_MASKDLG"), false, &pTestSettings->ShowMaskDlg);
	bRtn &= GetIntfromINIfile(_T("TEST_SETTINGS"), _T("LOOP_TEST "), 1, &pTestSettings->LoopTest);
	bRtn &= GetStringfromINIfile(_T("TEST_SETTINGS"), _T("AUTO_RUN_TRIGGER"), _T("DISABLE"), pTestSettings->AutoRunTrigger, MAX_PATH);

	return bRtn;
}

bool CParser::ParserDUTSettings(DUT_SETTINGS *pDutSettings)
{
	bool bRtn = true;

	bRtn &= GetChipTypefromINIfile(_T("DUT_SETTINGS"), _T("CHIP_TYPE"), CTYPE_UNKNOWN, &pDutSettings->chipType);
	bRtn &= GetTestTypefromINIfile(_T("DUT_SETTINGS"), _T("TEST_TYPE"), TTYPE_UNKNOWN, &pDutSettings->testType);
	bRtn &= GetStringfromINIfile(_T("DUT_SETTINGS"), _T("IP"), _T("192.168.1.1"), pDutSettings->IPAddr, IP_ADDRESS);
	bRtn &= GetIntfromINIfile(_T("DUT_SETTINGS"), _T("PORT "), 23, &pDutSettings->Port);
	bRtn &= GetStringfromINIfile(_T("DUT_SETTINGS"), _T("LOGIN_ID"), _T(""), pDutSettings->loginID, STRING_LENS_16);
	bRtn &= GetStringfromINIfile(_T("DUT_SETTINGS"), _T("LOGIN_PWD"), _T(""), pDutSettings->loginPwd, STRING_LENS_16);
	bRtn &= GetStringfromINIfile(_T("DUT_SETTINGS"), _T("LOGIN_ID_PROMPT"), _T(""), pDutSettings->loginIDPrompt, STRING_LENS_32);
	bRtn &= GetStringfromINIfile(_T("DUT_SETTINGS"), _T("LOGIN_PWD_PROMPT"), _T(""), pDutSettings->loginPwdPrompt, STRING_LENS_32);
	bRtn &= GetStringfromINIfile(_T("DUT_SETTINGS"), _T("SHELL_PROMPT"), _T("#"), pDutSettings->shellPrompt, STRING_LENS_16);
	bRtn &= GetBoolfromINIfile(_T("DUT_SETTINGS"), _T("WRITE_TO_EFUSE"), true, &pDutSettings->Write2EFuse);	
	bRtn &= GetBoolfromINIfile(_T("DUT_SETTINGS"), _T("IGNORE_CMD_RESPONSE"), false, &pDutSettings->IgnoreResponse);

	return bRtn;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

bool CParser::GetTestParameter(CString strParameter, CString strKey, int *Value)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((Value) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	bRtn = true;
	*Value = atoi(strValue.GetBuffer());

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

void CParser::GetTestParameter(CString strParameter, CString strKey, int *Value, int defaultValue)
{
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_End;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((Value) && (strValue.Compare(_T("")) != 0))) goto go_End;
	*Value = atoi(strValue);

	return ;
go_End:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s', Set default '%d'"), strKey, defaultValue);
	*Value = defaultValue;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, double *Value)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((Value) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	bRtn = true;
	*Value = atof(strValue.GetBuffer());

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, double Value[2], double *Step)
{
	bool bRtn = false;
	CString strValue, strStart, strStop, strStep;
	int nSIdx = -1, nEIdx = -1, nIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if((nIdx=strValue.Find(_T("~"))) == -1) goto go_Error;

	strStart = strValue.Mid(0, nIdx);
	strValue = strValue.Mid(nIdx+1);

	if((nIdx=strValue.Find(_T("/"))) == -1)
	{
		strStop = strValue.Mid(0);
		strStep = _T("1");
	}else
	{
		strStop = strValue.Mid(0, nIdx);
		strStep = strValue.Mid(nIdx+1);
	}

	Value[0] = atof(strStart.GetBuffer());
	Value[1] = atof(strStop.GetBuffer());
	*Step = atof(strStep.GetBuffer());
	bRtn = true;

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, LPSTR Value, int nLength)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((Value) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	bRtn = true;
	strcpy_s(Value, nLength, strValue.GetBuffer());

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, LPSTR Value, int nLength, int *nRout)
{
	bool bRtn = false;
	CString strValue, strIP, strRout;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((Value) && (strValue.Compare(_T("")) != 0))) goto go_Error;

	//Find the Rout
	if((nSIdx=strValue.Find(_T(":"))) != -1)
	{
		strIP = strValue.Mid(0, nSIdx);
		strcpy_s(Value, nLength, strIP.GetBuffer());

		strRout = strValue.Mid(nSIdx+1);		//1(A): ROUT1, 2(B): ROUT2	
		if(strRout.CompareNoCase(_T("A")) == 0) *nRout = 1;
		else if(strRout.CompareNoCase(_T("B")) == 0) *nRout = 2;
		else goto go_BadFormat;
		
	}else 
	{
		strcpy_s(Value, nLength, strValue.GetBuffer());
		*nRout = 1;
	}

	bRtn = true;
	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return false;
go_BadFormat:
	CCommFunc::OutputTracelog(_T("Incorrect ROUT format '%s'"), strRout);
	return false;
}


bool CParser::GetTestParameter(CString strParameter, CString strKey, bool *Value)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((Value) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	else
	{
		if((strValue.Compare(_T("TRUE")) == 0) || (strValue.Compare(_T("true")) == 0)) *Value = true;
		else if((strValue.Compare(_T("FALSE")) == 0) || (strValue.Compare(_T("false")) == 0)) *Value = false;
		else bRtn = false;

		bRtn = true;
	}	

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, RF_TYPE *rfType)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((rfType) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	else
	{
		if(strValue.Compare(_T("NI")) == 0) *rfType = RF_NI;
		else if(strValue.Compare(_T("IQXEL80")) == 0) *rfType = RF_IQXEL_80;
		else if(strValue.Compare(_T("IQXEL-M8")) == 0) *rfType = RF_IQXEL_M8;
		else if(strValue.Compare(_T("IQXEL-M2W")) == 0) *rfType = RF_IQXEL_M2W;
		else *rfType = RF_IQXEL_80;

		bRtn = true;
	}	

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, IQAPI_PORT_ENUM *MeasurePort)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((MeasurePort) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	else
	{
		if(strValue.Compare(_T("LEFT")) == 0) *MeasurePort = PORT_LEFT;
		else if(strValue.Compare(_T("RIGHT")) == 0) *MeasurePort = PORT_RIGHT;
		else if(strValue.Compare(_T("1")) == 0) *MeasurePort = PORT_1;		
		else if(strValue.Compare(_T("2")) == 0) *MeasurePort = PORT_2;
		else if(strValue.Compare(_T("3")) == 0) *MeasurePort = PORT_3;
		else if(strValue.Compare(_T("4")) == 0) *MeasurePort = PORT_4;
		else goto go_Error;

		bRtn = true;
	}	

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, ANALYZE_TYPE *AnalyzeType)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((AnalyzeType) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	else
	{
		if(strValue.Compare(_T("80211_B_CAL")) == 0) *AnalyzeType = STANDARD_802_11_B_CAL;
		else if(strValue.Compare(_T("80211_B")) == 0) *AnalyzeType = STANDARD_802_11_B;
		else if(strValue.Compare(_T("80211_AG")) == 0) *AnalyzeType = STANDARD_802_11_AG;
		else if(strValue.Compare(_T("80211_AG_CAL")) == 0) *AnalyzeType = STANDARD_802_11_AG_CAL;
		else if(strValue.Compare(_T("80211_N")) == 0) *AnalyzeType = STANDARD_802_11_N;
		else if(strValue.Compare(_T("80211_N_CAL")) == 0) *AnalyzeType = STANDARD_802_11_N_CAL;
		else if(strValue.Compare(_T("80211_AC")) == 0) *AnalyzeType = STANDARD_802_11_AC;
		else if(strValue.Compare(_T("80211_AC_CAL")) == 0) *AnalyzeType = STANDARD_802_11_AC_CAL;
		else if(strValue.Compare(_T("BLUETOOTH")) == 0) *AnalyzeType = STANDARD_BLUETOOTH;
		else if(strValue.Compare(_T("ZIGBEE")) == 0) *AnalyzeType = STANDARD_ZIGBEE;
		else goto go_Error;
		bRtn = true;
	}	

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, ANT_SELECTION *AntSel)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((AntSel) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	else
	{
		bRtn = true;

		if(strValue.Compare(_T("1")) == 0) *AntSel = TX_CHAIN_A;
		else if(strValue.Compare(_T("2")) == 0) *AntSel = TX_CHAIN_B;
		else if(strValue.Compare(_T("3")) == 0) *AntSel = TX_CHAIN_C;
		else if(strValue.Compare(_T("4")) == 0) *AntSel = TX_CHAIN_D;

		else if(strValue.Compare(_T("12")) == 0) *AntSel = TX_CHAIN_AB;
		else if(strValue.Compare(_T("123")) == 0) *AntSel = TX_CHAIN_ABC;
		else if(strValue.Compare(_T("1234")) == 0) *AntSel = TX_CHAIN_ALL;

		else *AntSel = TX_DONT_CARE;
	}	

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, BAND_WIDTH *BandWidth)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((BandWidth) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	else
	{
		if((strValue.Compare(_T("20M")) == 0) || (strValue.Compare(_T("20MHZ")) == 0)) *BandWidth = BW_20MHZ;
		else if((strValue.Compare(_T("40M")) == 0) || (strValue.Compare(_T("40MHZ")) == 0)) *BandWidth = BW_40MHZ;
		else if((strValue.Compare(_T("80M")) == 0) || (strValue.Compare(_T("80MHZ")) == 0)) *BandWidth = BW_80MHZ;
		else if((strValue.Compare(_T("160M")) == 0) || (strValue.Compare(_T("160MHZ")) == 0)) *BandWidth = BW_160MHZ;
		else goto go_Error;
		bRtn = true;
	}	

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

bool CParser::GetTestParameter(CString strParameter, CString strKey, DATA_RATE *DataRate)
{
	bool bRtn = false;
	CString strValue;
	int nSIdx = -1, nEIdx = -1, nLens = 0;

	if((nSIdx=strParameter.Find(strKey)) == -1) goto go_Error;
	nSIdx += strKey.GetLength()+1;

	nEIdx = strParameter.Find(_T(","), nSIdx+1);
	if(nEIdx == -1) nLens = strlen(strParameter) - nEIdx;
	else nLens = nEIdx - nSIdx;

	strValue = strParameter.Mid(nSIdx, nLens);
	if(!((DataRate) && (strValue.Compare(_T("")) != 0))) goto go_Error;
	else
	{
		/*802.11b*/
		if(strValue.Compare(_T("DSSS_1")) == 0) *DataRate = DATARATE_DSSS_1;
		else if(strValue.Compare(_T("DSSS_2")) == 0) *DataRate = DATARATE_DSSS_2;
		else if(strValue.Compare(_T("CCK_5_5")) == 0) *DataRate = DATARATE_CCK_5_5;
		else if(strValue.Compare(_T("CCK_11")) == 0) *DataRate = DATARATE_CCK_11;
		/*802.11ag*/
		else if(strValue.Compare(_T("OFDM_6")) == 0) *DataRate = DATARATE_OFDM_6;
		else if(strValue.Compare(_T("OFDM_9")) == 0) *DataRate = DATARATE_OFDM_9;
		else if(strValue.Compare(_T("OFDM_12")) == 0) *DataRate = DATARATE_OFDM_12;
		else if(strValue.Compare(_T("OFDM_18")) == 0) *DataRate = DATARATE_OFDM_18;
		else if(strValue.Compare(_T("OFDM_24")) == 0) *DataRate = DATARATE_OFDM_24;
		else if(strValue.Compare(_T("OFDM_36")) == 0) *DataRate = DATARATE_OFDM_36;
		else if(strValue.Compare(_T("OFDM_48")) == 0) *DataRate = DATARATE_OFDM_48;
		else if(strValue.Compare(_T("OFDM_54")) == 0) *DataRate = DATARATE_OFDM_54;
		/*802.11n*/
		else if(strValue.Compare(_T("MCS0")) == 0) *DataRate = DATARATE_MCS0;
		else if(strValue.Compare(_T("MCS1")) == 0) *DataRate = DATARATE_MCS1;
		else if(strValue.Compare(_T("MCS2")) == 0) *DataRate = DATARATE_MCS2;
		else if(strValue.Compare(_T("MCS3")) == 0) *DataRate = DATARATE_MCS3;
		else if(strValue.Compare(_T("MCS4")) == 0) *DataRate = DATARATE_MCS4;
		else if(strValue.Compare(_T("MCS5")) == 0) *DataRate = DATARATE_MCS5;
		else if(strValue.Compare(_T("MCS6")) == 0) *DataRate = DATARATE_MCS6;
		else if(strValue.Compare(_T("MCS7")) == 0) *DataRate = DATARATE_MCS7;
		else if(strValue.Compare(_T("MCS8")) == 0) *DataRate = DATARATE_MCS8;
		else if(strValue.Compare(_T("MCS9")) == 0) *DataRate = DATARATE_MCS9;
		else if(strValue.Compare(_T("MCS10")) == 0) *DataRate = DATARATE_MCS10;
		else if(strValue.Compare(_T("MCS11")) == 0) *DataRate = DATARATE_MCS11;
		else if(strValue.Compare(_T("MCS12")) == 0) *DataRate = DATARATE_MCS12;
		else if(strValue.Compare(_T("MCS13")) == 0) *DataRate = DATARATE_MCS13;
		else if(strValue.Compare(_T("MCS14")) == 0) *DataRate = DATARATE_MCS14;
		else if(strValue.Compare(_T("MCS15")) == 0) *DataRate = DATARATE_MCS15;
		else if(strValue.Compare(_T("MCS16")) == 0) *DataRate = DATARATE_MCS16;
		else if(strValue.Compare(_T("MCS17")) == 0) *DataRate = DATARATE_MCS17;
		else if(strValue.Compare(_T("MCS18")) == 0) *DataRate = DATARATE_MCS18;
		else if(strValue.Compare(_T("MCS19")) == 0) *DataRate = DATARATE_MCS19;
		else if(strValue.Compare(_T("MCS20")) == 0) *DataRate = DATARATE_MCS20;
		else if(strValue.Compare(_T("MCS21")) == 0) *DataRate = DATARATE_MCS21;
		else if(strValue.Compare(_T("MCS22")) == 0) *DataRate = DATARATE_MCS22;
		else if(strValue.Compare(_T("MCS23")) == 0) *DataRate = DATARATE_MCS23;
		else if(strValue.Compare(_T("MCS24")) == 0) *DataRate = DATARATE_MCS24;
		else if(strValue.Compare(_T("MCS25")) == 0) *DataRate = DATARATE_MCS25;
		else if(strValue.Compare(_T("MCS26")) == 0) *DataRate = DATARATE_MCS26;
		else if(strValue.Compare(_T("MCS27")) == 0) *DataRate = DATARATE_MCS27;
		else if(strValue.Compare(_T("MCS28")) == 0) *DataRate = DATARATE_MCS28;
		else if(strValue.Compare(_T("MCS29")) == 0) *DataRate = DATARATE_MCS29;
		else if(strValue.Compare(_T("MCS30")) == 0) *DataRate = DATARATE_MCS30;
		else if(strValue.Compare(_T("MCS31")) == 0) *DataRate = DATARATE_MCS31;
		/*Bluetooth BDR*/
		else if(strValue.Compare(_T("1DH1")) == 0) *DataRate = DATARATE_BT_1DH1;
		else if(strValue.Compare(_T("1DH3")) == 0) *DataRate = DATARATE_BT_1DH3;
		else if(strValue.Compare(_T("1DH5")) == 0) *DataRate = DATARATE_BT_1DH5;
		/*Bluetooth EDR*/
		else if(strValue.Compare(_T("2DH1")) == 0) *DataRate = DATARATE_BT_2DH1;
		else if(strValue.Compare(_T("2DH3")) == 0) *DataRate = DATARATE_BT_2DH3;
		else if(strValue.Compare(_T("2DH5")) == 0) *DataRate = DATARATE_BT_2DH5;
		else if(strValue.Compare(_T("3DH1")) == 0) *DataRate = DATARATE_BT_3DH1;
		else if(strValue.Compare(_T("3DH3")) == 0) *DataRate = DATARATE_BT_3DH3;
		else if(strValue.Compare(_T("3DH5")) == 0) *DataRate = DATARATE_BT_3DH5;
		else goto go_Error;	
		bRtn = true;
	}	

	return bRtn;
go_Error:
	CCommFunc::OutputTracelog(_T("Fail to parser '%s'"), strKey);
	return bRtn;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

bool CParser::ParserPathloss(LPSTR lpszFile, CLogger *pLogger)
{
	TCHAR *pLine = new TCHAR[STRING_LENS_256], *pCh;
	int fieldCnt = 0;
	ifstream fStream(lpszFile);

	if(m_PathlossAry.GetSize()) goto go_End;
	memset(pLine, _T('\0'), STRING_LENS_256);

	while(fStream.getline(pLine, STRING_LENS_256))
	{
		PATHLOSS_ITEM *pItem = new PATHLOSS_ITEM;
		memset(pItem, 0, sizeof(PATHLOSS_ITEM));

		fieldCnt = 0;
		pCh = strtok(pLine, _T(","));
		while(pCh != NULL)
		{
			if(fieldCnt == 0)
			{
				pItem->Frequency = atoi(pCh);

				if(pItem->Frequency <= 2472) pItem->Channel = (int)(pItem->Frequency-2407)/5;
				else pItem->Channel = (int) (((double)(pItem->Frequency-5000)/10)*2);
			}else
			{
				if(fieldCnt <= MAX_PATHLOSS_CNT) pItem->Pathloss[fieldCnt-1] = atof(pCh);
			}

			/*Next Field*/
			fieldCnt++;
			pCh = strtok(NULL, _T(","));
		}

		m_PathlossAry.Add(pItem);
	}
	
go_End:
	if(pLogger && m_PathlossAry.GetSize()) DumpPathloss(pLogger);
	DEL_ARRAY(pLine)
	return (m_PathlossAry.GetSize());
}

bool CParser::GetPathlossByCH(int Channel, ANT_SELECTION AntSel, double *Pathloss, double AllPathloss[])
{
	bool bRtn = false;
	int nAntIdx = -1, nTail = 0, nFront = 0;
	*Pathloss = 0;

	for(int n=0 ; n<m_PathlossAry.GetSize() ; n++)
	{
		PATHLOSS_ITEM *pItem = m_PathlossAry.GetAt(n);

		if(pItem->Channel == Channel)
		{
			if((AntSel == TX_CHAIN_AB) || (AntSel == TX_CHAIN_ABC) || (AntSel == TX_CHAIN_ALL))
			{
				AllPathloss[0] = pItem->Pathloss[0];
				AllPathloss[1] = pItem->Pathloss[1];
				AllPathloss[2] = pItem->Pathloss[2];
				AllPathloss[3] = pItem->Pathloss[3];

				bRtn = true;
				goto go_End;
			}else
			{
				if((nAntIdx = ConvertAnt2Idx(AntSel)) != -1)
				{
					AllPathloss[0] = pItem->Pathloss[0];
					AllPathloss[1] = pItem->Pathloss[1];
					AllPathloss[2] = pItem->Pathloss[2];
					AllPathloss[3] = pItem->Pathloss[3];

					*Pathloss = pItem->Pathloss[nAntIdx];
					bRtn = true;
					goto go_End;
				}
			}			
		}
	}

go_End:
	if(!bRtn) CCommFunc::OutputTracelog(_T("Fail to get the pathloss by CH=%d"), Channel);
	return bRtn;
}

void CParser::DumpPathloss(CLogger *pLogger)
{
	double dbDiff = 0, dbMax = -999, dbMin = 999;
	CString strDump = _T(""), strTmp;

	if(!pLogger || (m_PathlossAry.GetSize() == 0)) return;

	pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, _T("%-5s   %3s\t\t%5s  %5s  %5s  %5s\t%-5s\r\n__________________________________________________________\r\n"), 
		_T("Freq"), _T("CH"), _T("Ant.A"), _T("Ant.B"), _T("Ant.C"), _T("Ant.D"), _T("Diff"));
	for(int n=0 ; n<m_PathlossAry.GetSize() ; n++)
	{
		PATHLOSS_ITEM *pItem = m_PathlossAry.GetAt(n);

		dbDiff = 0, dbMax = -999, dbMin = 999;
		FindDiff(pItem, &dbMax, &dbMin, &dbDiff);

		strTmp.Format(_T("%-5d [%3d]\t\t%2.2f  %2.2f  %2.2f  %2.2f\t%1.1fdB"), pItem->Frequency, pItem->Channel, 
			((pItem->Pathloss[0]==NA_NUMBER)?(0.0):pItem->Pathloss[0]), 
			((pItem->Pathloss[1]==NA_NUMBER)?(0.0):pItem->Pathloss[1]), 
			((pItem->Pathloss[2]==NA_NUMBER)?(0.0):pItem->Pathloss[2]), 
			((pItem->Pathloss[3]==NA_NUMBER)?(0.0):pItem->Pathloss[3]), dbDiff); 

		if(n != m_PathlossAry.GetSize()-1) strDump += (strTmp + _T("\r\n"));
		else strDump += strTmp;
	}

	pLogger->OutputLog(TYPE_NORMAL, COLOR_WHITE, strDump.GetBuffer());
}

void CParser::FindDiff(PATHLOSS_ITEM *pItem, double *dbMax, double *dbMin, double *dbDiff)
{
	for(int n=0 ; n<MAX_PATHLOSS_CNT ; n++)
	{
		if((pItem->Pathloss[n] == NA_NUMBER) || (pItem->Pathloss[n] == 0)) continue;
		if(pItem->Pathloss[n] > *dbMax) *dbMax = pItem->Pathloss[n];
		if(pItem->Pathloss[n] < *dbMin) *dbMin = pItem->Pathloss[n];
	}

	*dbDiff = *dbMax - *dbMin;
}

int CParser::ConvertAnt2Idx(ANT_SELECTION AntSel)
{
	int nIdx = -1;

	if(AntSel == TX_CHAIN_A) nIdx = 0;
	else if(AntSel == TX_CHAIN_B) nIdx = 1;
	else if(AntSel == TX_CHAIN_C) nIdx = 2;
	else if(AntSel == TX_CHAIN_D) nIdx = 3;

	return nIdx;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

bool CParser::GetStringfromINIfile(LPSTR lpSession, LPSTR lpKey, LPSTR lpDefault, LPSTR rtResult, int BufferLens)
{
	bool bRtn = false;
	DWORD dwRtn;
	TCHAR *Buffer = NULL;

	if((Buffer = new TCHAR[STRING_LENS_2048]))
	{
		memset(Buffer, 0, STRING_LENS_2048);

		if(!(GetPrivateProfileString(lpSession, lpKey, lpDefault, Buffer, STRING_LENS_2048, m_strTFFile.GetBuffer())))
		{
			if(lpDefault) strcpy_s(rtResult, BufferLens, lpDefault);
			goto go_End;
		}

		CCommFunc::StringTrim(Buffer);
		strcpy_s(rtResult, BufferLens, Buffer);		
	}

go_End:
	DEL_ARRAY(Buffer)
	return bRtn;
}

bool CParser::GetTestTypefromINIfile(LPSTR lpSession, LPSTR lpKey, TESTTYPE ttDefaulut, TESTTYPE *ttResult)
{
	bool bRtn = false;
	TCHAR *Buffer = NULL;

	if((Buffer = new TCHAR[STRING_LENS_2048]))
	{
		memset(Buffer, 0, STRING_LENS_2048);
		if(!(GetPrivateProfileString(lpSession, lpKey, _T(""), Buffer, STRING_LENS_2048, m_strTFFile.GetBuffer())))
		{
			*ttResult = ttDefaulut;
			goto go_End;
		}else bRtn = true;

		CCommFunc::StringTrim(Buffer);

		if((strcmp(Buffer, _T("API")) == 0) || (strcmp(Buffer, _T("api")) == 0)) *ttResult = TTYPE_API;
		else if((strcmp(Buffer, _T("TELNET")) == 0) || (strcmp(Buffer, _T("telnet")) == 0)) *ttResult = TTYPE_TELNET;
		else if((strcmp(Buffer, _T("SOCKET")) == 0) || (strcmp(Buffer, _T("socket")) == 0)) *ttResult = TTYPE_SOCKET;
		else *ttResult = TTYPE_UNKNOWN;
	}

go_End:
	DEL_ARRAY(Buffer)
	return bRtn;
}

bool CParser::GetChipTypefromINIfile(LPSTR lpSession, LPSTR lpKey, CHIPTYPE ctDefault, CHIPTYPE *ctResult)
{
	bool bRtn = false;
	TCHAR *Buffer = NULL;

	if((Buffer = new TCHAR[STRING_LENS_2048]))
	{
		memset(Buffer, 0, STRING_LENS_2048);
		if(!(GetPrivateProfileString(lpSession, lpKey, _T(""), Buffer, STRING_LENS_2048, m_strTFFile.GetBuffer())))
		{
			*ctResult = ctDefault;
			goto go_End;
		}else bRtn = true;

		CCommFunc::StringTrim(Buffer);

		if(strcmp(Buffer, _T("BCM_4708")) == 0) *ctResult = CTYPE_BCM_4708;
		else if(strcmp(Buffer, _T("MTK_7668")) == 0) *ctResult = CTYPE_MTK_7668;
		else *ctResult = CTYPE_UNKNOWN;
	}

go_End:
	DEL_ARRAY(Buffer)
	return bRtn;
}

bool CParser::GetIntfromINIfile(LPSTR lpSession, LPSTR lpKey, int nDefault, int *nResult)
{
	bool bRtn = false;
	TCHAR *Buffer = NULL;

	if((Buffer = new TCHAR[STRING_LENS_2048]))
	{
		memset(Buffer, 0, STRING_LENS_2048);
		if(!(GetPrivateProfileString(lpSession, lpKey, _T(""), Buffer, STRING_LENS_2048, m_strTFFile.GetBuffer())))
		{
			*nResult = nDefault;
			goto go_End;
		}else bRtn = true;

		CCommFunc::StringTrim(Buffer);

		*nResult = atoi(Buffer);
	}

go_End:
	DEL_ARRAY(Buffer)
	return bRtn;
}

bool CParser::GetBoolfromINIfile(LPSTR lpSession, LPSTR lpKey, bool bDefault, bool *bResult)
{
	bool bRtn = false;
	TCHAR *Buffer = NULL;

	if((Buffer = new TCHAR[STRING_LENS_2048]))
	{
		memset(Buffer, 0, STRING_LENS_2048);
		if(!(GetPrivateProfileString(lpSession, lpKey, bDefault?_T("true"):_T("false"), Buffer, STRING_LENS_2048, m_strTFFile.GetBuffer())))
		{
			*bResult = bDefault;
			goto go_End;
		}else bRtn = true;

		CCommFunc::StringTrim(Buffer);

		if((strcmp(Buffer, _T("true")) == 0) || (strcmp(Buffer, _T("TRUE")) == 0)) *bResult = true;
		else if((strcmp(Buffer, _T("false")) == 0) || (strcmp(Buffer, _T("FALSE")) == 0)) *bResult = false;
		else *bResult = bDefault;
	}

go_End:
	DEL_ARRAY(Buffer)
	return bRtn;
}