#pragma once

#include "..\..\..\..\Lib\rfDefine.h"

#include "..\Logger\Logger.h"

#define MAX_PATHLOSS_CNT		4
#define MAX_PATHLOSS_DIFF		0.5

typedef struct _PATHLOSS_ITEM
{
	int Channel;
	int Frequency;
	double Pathloss[MAX_PATHLOSS_CNT];
}PATHLOSS_ITEM, *PPATHLOSS_ITEM;
typedef CArray<PPATHLOSS_ITEM, PPATHLOSS_ITEM> PATHLOSSARY;

class CParser
{
public:
	CParser(void);
	~CParser(void);

public:
	void InitializeParser(LPSTR lpszFile, CLogger *pLogger);
	void TerminateParser();

	/*Pathloss*/
	bool ParserPathloss(LPSTR lpszFile, CLogger *pLogger);
	bool GetPathlossByCH(int Channel, ANT_SELECTION AntSel, double *Pathloss, double AllPathloss[]);

	/*Test Item*/
	bool ParserTestItem(CStringArray *pTestFlowAry,TESTITEMARY *pTestItemAry);
	void ParserTestParameter(TEST_ITEM *pItem, CString strLine);

	bool ParserTestSettings(TEST_SETTINGS *pTestSettings);
	bool ParserDUTSettings(DUT_SETTINGS *pDutSettings);

	//Test Parsmeter/SPEC/Option
	bool GetTestParameter(CString strParameter, CString strKey, int *Value);
	void GetTestParameter(CString strParameter, CString strKey, int *Value, int defaultValue);
	bool GetTestParameter(CString strParameter, CString strKey, double *Value);
	bool GetTestParameter(CString strParameter, CString strKey, bool *Value);
	bool GetTestParameter(CString strParameter, CString strKey, LPSTR Value, int nLength);
	bool GetTestParameter(CString strParameter, CString strKey, LPSTR Value, int nLength, int *nRout);
	bool GetTestParameter(CString strParameter, CString strKey, DATA_RATE *DataRate);
	bool GetTestParameter(CString strParameter, CString strKey, RF_TYPE *rfType);
	bool GetTestParameter(CString strParameter, CString strKey, IQAPI_PORT_ENUM *MeasurePort);
	bool GetTestParameter(CString strParameter, CString strKey, ANALYZE_TYPE *AnalyzeType);
	bool GetTestParameter(CString strParameter, CString strKey, ANT_SELECTION *AntSel);
	bool GetTestParameter(CString strParameter, CString strKey, BAND_WIDTH *BandWidth);
	/*Rx sweep*/
	bool GetTestParameter(CString strParameter, CString strKey, double Value[2], double *Step);

	//INI parser
	bool GetStringfromINIfile(LPSTR lpSession, LPSTR lpKey, LPSTR lpDefault, LPSTR rtResult, int BufferLens);
	bool GetTestTypefromINIfile(LPSTR lpSession, LPSTR lpKey, TESTTYPE ttDefaulut, TESTTYPE *ttResult);
	bool GetChipTypefromINIfile(LPSTR lpSession, LPSTR lpKey, CHIPTYPE ctDefault, CHIPTYPE *ctResult);
	bool GetIntfromINIfile(LPSTR lpSession, LPSTR lpKey, int nDefault, int *nResult);
	bool GetBoolfromINIfile(LPSTR lpSession, LPSTR lpKey, bool bDefault, bool *bResult);

private:
	void DumpPathloss(CLogger *pLogger);
	void FindDiff(PATHLOSS_ITEM *pItem, double *dbMax, double *dbMin, double *dbDiff);
	int ConvertAnt2Idx(ANT_SELECTION AntSel);

	CLogger				*m_pLogger;
	PATHLOSSARY			m_PathlossAry;
	CString				m_strTFFile;

};

