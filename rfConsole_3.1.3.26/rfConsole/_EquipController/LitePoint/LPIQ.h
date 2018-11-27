#pragma once
#include "..\EquipCtrlBase.h"

#define LP_BUFFER_SIZE				8192
#define MAX_POSITIVE_SECTION		5
#define MAX_NEGATIVE_SECTION		5
#define MAX_BUFFER_SIZE_1			8192
#define MAX_BUFFER_SIZE				4096

typedef struct _INFO
{
	TCHAR szIQmeasure[STRING_LENS_64];
	TCHAR szSN[STRING_LENS_64];
	TCHAR szModel[STRING_LENS_64];
	TCHAR szFWversion[STRING_LENS_64];
	TCHAR szHWversion[STRING_LENS_64];
}INFO, *PINFO;

class CLPIQ : public EquipCtrlbase
{
public:
	CLPIQ(void);
	~CLPIQ(void);

public:
	void InitializeEquip(CLogger *pLogger);
	void TerminateEquip();

	bool ConnectDevice(void *dataStruct, LPSTR szRespMsg, int nLens);

	bool GetLock(LPSTR szRespMsg, int nLens);
	bool ReleaseLock(LPSTR szRespMsg, int nLens);

	bool SetupVSG(void *dataStruct, LPSTR szRespMsg, int nLens);
	bool SetupVSA(void *dataStruct, LPSTR szRespMsg, int nLens);

	bool StartMeasure(void *dataStruct, LPSTR szRespMsg, int nLens);
	bool StartGenerate(int frameCnt, LPSTR szRespMsg, int nLens);

	bool ReleaseCtrlDevice(LPSTR szRespMsg, int nLens);
	bool DisconnectDevice(LPSTR szRespMsg, int nLens);

	bool					m_Initialized;

	/*Only for IQ*/
	void SetAnalyzeSetting(IQANALYZE_SETTING *pIQAnalySetting);
	void SetupSpectrumMask(HWND *phSpectrumMask);

	/*Information*/
	INFO					m_DeviceInfo;

private:
	void ParserDeviceInfo(LPSTR lpInfoString);
	int Analyze80211b(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens);
	int Analyze80211ag(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens);
	int Analyze80211n(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens);
	int Analyze80211ac(WIFI_TXRESULT *pWifiTxResult, LPSTR szRespMsg, int nLens);

	int AnalyzeZigbee(ZB_RESULT *pZBResult, LPSTR szRespMsg, int nLens);
	int AnalyzeBluetooth(BT_RESULT *pBTResult, LPSTR szRespMsg, int nLens);

	CLogger					*m_pLogger;
	
	RF_TYPE					m_rfType;
	IQAPI_PORT_ENUM			m_MeasurePort;
	double					m_samplingRate;
	
	IQANALYZE_SETTING		m_IQAnalySetting;
	IQVSA_SETTINGS			m_IQVSASettings;
	IQVSG_SETTINGS			m_IQVSGSettings;

	bool						m_bVSGOnOff;
	int						m_numSegm;

	HWND						*m_phSpectrumMask;
};

