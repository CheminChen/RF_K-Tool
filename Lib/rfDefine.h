#pragma once
#ifndef _RFCONSOLE_DEFINE_H__
#define _RFCONSOLE_DEFINE_H__

#include <time.h>
#include <fstream> 
#include<conio.h>

#include ".\_Import\IQmeasure_3.1.3.26\Include\IQmeasure.h"

using namespace std;

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

#define LOOP_INTERVAL_DELAY					1000
#define MD5_LENGTH							32+1

#define IP_ADDRESS							16
#define STRING_LENS_16						16
#define STRING_LENS_32						32
#define STRING_LENS_64						64
#define STRING_LENS_256						256
#define STRING_LENS_1024						1024
#define STRING_LENS_2048						2048
#define STRING_LENS_4096						4096
#define MAX_ARCBUFFER_SIZE					65535
#define UNDEFINE_VALUE						-9999

#define	EDITOR_INFO _T("RF test program v%s by JerryC")
#define GUI_WIN_NAME _T("Spectrum Mask")

#define CLEAN_CARRAY(TArray) while(TArray.GetSize()){delete TArray.GetAt(0);TArray.RemoveAt(0);} 
#define DEL_ITEM(ITEM) if(ITEM){delete ITEM; ITEM=NULL;}
#define DEL_ARRAY(ITEM) if(ITEM){delete []ITEM; ITEM=NULL;}
#define ROUND(x) ((x>0)?(int)(x+0.5):(int)(x-0.5))

#define WM_SPECTRUM_MASK						WM_USER+101

//LOGGER
#define COLOR_WHITE							FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
#define COLOR_RED							FOREGROUND_RED
#define COLOR_GREEN							FOREGROUND_GREEN
#define COLOR_YELLOW							FOREGROUND_RED|FOREGROUND_GREEN
#define COLOR_MAGENTA						FOREGROUND_BLUE|FOREGROUND_GREEN

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

typedef enum
{
	TYPE_TITLE			 = 0,	
	TYPE_RESULT			 = 1,
	TYPE_SUMMARY		 = 2,
	TYPE_NORMAL          = 4,
	TYPE_DETAILS		 = 8
}LOGTYPE;

/*DUT_SETTINGS*/
typedef enum 
{
	CTYPE_BCM_4708,
	CTYPE_MTK_7668,
	CTYPE_BCM_4366,
	CTYPE_UNKNOWN
}CHIPTYPE;

typedef enum 
{
	TTYPE_API,
	TTYPE_TELNET,
	TTYPE_SOCKET,
	TTYPE_UNKNOWN
}TESTTYPE;

typedef struct _DUT_SETTINGS
{
	TCHAR Timestamp[STRING_LENS_32];
	TCHAR MAC[STRING_LENS_16];
	TCHAR SN[STRING_LENS_16];
	CHIPTYPE chipType;
	TESTTYPE testType;
	TCHAR IPAddr[IP_ADDRESS];
	int Port;
	TCHAR loginID[STRING_LENS_16];
	TCHAR loginPwd[STRING_LENS_16];
	TCHAR loginIDPrompt[STRING_LENS_32];
	TCHAR loginPwdPrompt[STRING_LENS_32];
	TCHAR shellPrompt[STRING_LENS_16];
	bool Write2EFuse;
	bool IgnoreResponse;
	TCHAR wifiIF[STRING_LENS_16];
}DUT_SETTINGS, *PDUT_SETTINGS;

/*TEST_SETTINGS*/
typedef struct _TEST_SETTINGS
{
	TCHAR actualMD5[MD5_LENGTH];
	bool EnableDebugMsg;
	bool StopOnFail;
	bool ExportCSV;
	bool ShowMaskDlg;
	int LoopTest;
	TCHAR AutoRunTrigger[MAX_PATH];
}TEST_SETTINGS, *PTEST_SETTINGS;

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

typedef enum
{
	STATUS_PASS,
	STATUS_SPEC_FAIL,
	STATUS_PARSER_FAIL,
	STATUS_SKIP,
	STATUS_FUNCTION_ERROR,	
	STATUS_DEVICE_ERROR,
	STATUS_UNDEFINE
}RUNSTATUS;

typedef enum
{
	MODE_RUN = 0,
	MODE_SKIP = 1,
	MODE_FIXED = 2,
	MODE_UNDEFINE = -9999
}RUN_MODE;

typedef struct _TEST_ITEM
{
	int TestIdx;
	TCHAR TestCase[STRING_LENS_64];
	TCHAR TestItem[STRING_LENS_64];
	TCHAR TestParameter[STRING_LENS_2048];
	TCHAR TestSPEC[STRING_LENS_256];
	TCHAR TestOption[STRING_LENS_256];
	RUN_MODE RunMode;
	bool TestResult;
	double TestTime;
	int IntervalSleep;
	inline _TEST_ITEM()
	{
		TestIdx = 0;
		memset(TestCase, '\0', STRING_LENS_64);
		memset(TestItem, '\0', STRING_LENS_64);
		memset(TestParameter, '\0', STRING_LENS_2048);
		memset(TestSPEC, '\0', STRING_LENS_256);
		memset(TestOption, '\0', STRING_LENS_256);
		RunMode = MODE_UNDEFINE;
		TestResult = true;
		TestTime = 0;
		IntervalSleep = 0;
	}
}TEST_ITEM, *PTEST_ITEM;
typedef CArray<PTEST_ITEM, PTEST_ITEM> TESTITEMARY;

typedef struct _TEST_RESULT
{
	bool TestResult;
	char ErrorCode[STRING_LENS_32];
	char ErrorItem[STRING_LENS_64];
	char ErrorMessage[STRING_LENS_4096];

	double TestTime;
	int TotalRun;
	int PassedRun;
}TEST_RESULT, *PTEST_RESULT;

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

#define NA_NUMBER				-99999.99

typedef enum
{
	TRIM_POWER,
	POWER_CAL,
	XTAL_CAL,
	RX_CAL
}CAL_TYPE;

typedef enum
{
	RF_TX,
	RF_RX
}RF_MODE;

typedef enum 
{
	TX_CHAIN_A   = 1,
	TX_CHAIN_B   = 2,
	TX_CHAIN_C   = 4,
	TX_CHAIN_D   = 8,
	TX_CHAIN_AB  = TX_CHAIN_A | TX_CHAIN_B,
	TX_CHAIN_AC  = TX_CHAIN_A | TX_CHAIN_C,
	TX_CHAIN_BC  = TX_CHAIN_B | TX_CHAIN_C,
	TX_CHAIN_ABC = TX_CHAIN_A | TX_CHAIN_B | TX_CHAIN_C, 
	TX_CHAIN_ALL = TX_CHAIN_A | TX_CHAIN_B | TX_CHAIN_C | TX_CHAIN_D,
	TX_DONT_CARE    = 9999
}ANT_SELECTION;

typedef enum
{
	BW_20MHZ,
	BW_40MHZ,
	BW_80MHZ,
	BW_160MHZ,
	BW_80_80MHZ
}BAND_WIDTH;

typedef enum
{
	MODULATION_CW,
	MODULATION_DSSS,
	MODULATION_OFDM
}MODULATION_TYPE;

typedef enum
{
	STANDARD_802_11_B,
	STANDARD_802_11_B_CAL,
	STANDARD_802_11_AG,
	STANDARD_802_11_AG_CAL,
	STANDARD_802_11_N,
	STANDARD_802_11_N_CAL,
	STANDARD_802_11_AC,
	STANDARD_802_11_AC_CAL,
	STANDARD_BLUETOOTH,
	STANDARD_ZIGBEE
}ANALYZE_TYPE;

typedef enum
{
	//802.11b
	DATARATE_DSSS_1, DATARATE_DSSS_2, DATARATE_CCK_5_5, DATARATE_CCK_11,                     
	//802.11ag
	DATARATE_OFDM_6, DATARATE_OFDM_9, DATARATE_OFDM_12, DATARATE_OFDM_18,                  
	DATARATE_OFDM_24, DATARATE_OFDM_36, DATARATE_OFDM_48, DATARATE_OFDM_54,                    
	//802.11n
	DATARATE_MCS0, DATARATE_MCS1, DATARATE_MCS2, DATARATE_MCS3,                       
	DATARATE_MCS4, DATARATE_MCS5, DATARATE_MCS6, DATARATE_MCS7,                       
	DATARATE_MCS8, DATARATE_MCS9, DATARATE_MCS10, DATARATE_MCS11,                       
	DATARATE_MCS12, DATARATE_MCS13, DATARATE_MCS14, DATARATE_MCS15,                       
	DATARATE_MCS16, DATARATE_MCS17, DATARATE_MCS18, DATARATE_MCS19,                       
	DATARATE_MCS20, DATARATE_MCS21, DATARATE_MCS22, DATARATE_MCS23,                       
	DATARATE_MCS24, DATARATE_MCS25, DATARATE_MCS26, DATARATE_MCS27,                      
	DATARATE_MCS28, DATARATE_MCS29, DATARATE_MCS30, DATARATE_MCS31, 
	//Bluetooth BDR
	DATARATE_BT_1DH1, DATARATE_BT_1DH3, DATARATE_BT_1DH5,                    
	//Bluetooth EDR
	DATARATE_BT_2DH1, DATARATE_BT_2DH3, DATARATE_BT_2DH5,                  
	DATARATE_BT_3DH1, DATARATE_BT_3DH3, DATARATE_BT_3DH5      
}DATA_RATE;

typedef struct _RF_SETTINGS
{
	int Channel;
	double Frequency;
	ANT_SELECTION AntSel;	
	double TxPower;
	double RxPower;
	double SweepPower[2];
	double SweepStep;
	DATA_RATE DataRate;
	MODULATION_TYPE Modulation;
	BAND_WIDTH BandWidth;
	ANALYZE_TYPE AnalyzeType;
	double Pathloss;

	//For Calibration
	double PwrCalLimit;
	double RxGainLimit;
	int AntCalNum;
	int BandCalNum;
	TCHAR szModFile[MAX_PATH];
	TCHAR szRefFile[MAX_PATH];
	double AllPathloss[4];

	int FreqOffset;
	//void *CustomizeData;
}RF_SETTINGS, *PRF_SETTINGS;

typedef struct _SPEC_SETTINGS
{
	/*Tx*/
	double PowerH, PowerL;
	double EVM;
	double FreqError;
	double SpectrumViolation;		/*Spectrum Mask Percent*/
	double LoLeakage;
	/*Rx*/
	double PER;
	double RssiH, RssiL;

	inline _SPEC_SETTINGS() {PowerH = PowerL = EVM = FreqError = SpectrumViolation = LoLeakage = PER = RssiH = RssiL = NA_NUMBER;}
}SPEC_SETTINGS, *PSPEC_SETTINGS;

typedef enum 
{
	RF_NI,
	RF_ITEST,
	//RF_IQ2010,	//No longer supported in v3.1.3.21
	RF_IQXEL_80,
	RF_IQXEL_M8,
	RF_IQXEL_M2W
}RF_TYPE;

typedef struct _IQDEV_SETTINGS
{
	RF_TYPE rfType;
	IQAPI_PORT_ENUM MeasurePort;
	int DUT_ID;
	int lockTimeoutSec;
	int IQxelConnectionType;

	//int DHPort;			//For IQ160/280
	//int TokenID;
	//int tokenTimeout;
	TCHAR IPAddr[IP_ADDRESS];
}IQDEV_SETTINGS, *PIQDEV_SETTINGS;

typedef struct _NIDEV_SETTINGS
{
	RF_TYPE rfType;
	int Port;			
	TCHAR IPAddr[IP_ADDRESS];
}NIDEV_SETTINGS, *PNIDEV_SETTINGS;

//TEST RESULT	
typedef struct _ZB_RESULT
{
	double evmAll, evmAllOffset, rxRmsPowerNoGapDbm, rxPeakPowerDbm, freqOffsetFineHz, symClockErrorPpm, phaseNoiseDegRmsAll, numSymbols, SpectrumMask;
	inline _ZB_RESULT() {evmAll = evmAllOffset = rxRmsPowerNoGapDbm = rxPeakPowerDbm = freqOffsetFineHz = symClockErrorPpm = phaseNoiseDegRmsAll = numSymbols = SpectrumMask = NA_NUMBER; }

}ZB_RESULT, *PZB_RESULT;

typedef struct _BT_RESULT
{
	double P_av_each_burst;
	double P_pk_each_burst;
	double btDataRate;

	//Low energy (BLE)
	double leFreqOffset, leDeltaF1Avg, leDeltaF2Max, leDeltaF2Avg, leFreqDevSyncAv, leFnMax, leDeltaF0FnMax, leDeltaF1F0, leDeltaFnFn_5Max, leIsCrcOk, lePduLength, leCount;
	//Legacy
	double freq_deviation, deltaF2AvAccess, deltaF2MaxAccess, freq_est, freq_estHeader;
	//BDR
	double freq_drift, deltaF1Average, deltaF2Average, deltaF2Max, maxfreqDriftRate;
	//EDR
	double EdrEVMAv, EdrEVMpk, EdrEVMvsTime, EdrPowDiffdB, EdrOmegaI, EdrExtremeOmega0, EdrExtremeOmegaI0;

	inline _BT_RESULT() 
	{ 
		P_av_each_burst = P_pk_each_burst = btDataRate = NA_NUMBER;
		leFreqOffset = leDeltaF1Avg = leDeltaF2Max = leDeltaF2Avg = leFreqDevSyncAv = leFnMax = leDeltaF0FnMax = leDeltaF1F0 = leDeltaFnFn_5Max = leIsCrcOk = lePduLength = leCount = NA_NUMBER;
		freq_deviation = deltaF2AvAccess = deltaF2MaxAccess = freq_est = freq_estHeader = NA_NUMBER;
		freq_drift = deltaF1Average = deltaF2Average = deltaF2Max = maxfreqDriftRate = NA_NUMBER;
		EdrEVMAv = EdrEVMpk = EdrEVMvsTime = EdrPowDiffdB = EdrOmegaI = EdrExtremeOmega0 = EdrExtremeOmegaI0 = NA_NUMBER;
	}

}BT_RESULT, *PBT_RESULT;

typedef struct _WIFI_TXRESULT
{
	TCHAR TestItem[STRING_LENS_64];
	double DataRate, evmAll, codingRate, phaseErr, freqErr, clockErr, rmsPower, loLeakageDb, SpectrumMask, SpectFlatness;
	double CompositePower[4];
	double CompositeEVM[4];
	inline _WIFI_TXRESULT() 
	{
		memset(&TestItem[0], 0, sizeof(TestItem));
		for(int n=0 ; n<4 ; n++)
		{
			CompositePower[n] = CompositeEVM[n] = NA_NUMBER;
		}
		DataRate = evmAll = codingRate = phaseErr = freqErr = clockErr = rmsPower = loLeakageDb = SpectrumMask = SpectFlatness = NA_NUMBER;
	}

}WIFI_TXRESULT, *PWIFI_TXRESULT;

typedef struct _WIFI_RXRESULT
{
	double PER, RSSI;
	inline _WIFI_RXRESULT()	{PER = RSSI = NA_NUMBER;}

}WIFI_RXRESULT, *PWIFI_RXRESULT;

typedef struct _MEASURE_RESULT
{
	ANALYZE_TYPE AnalyzeType;
	void *TestResult;
}MEASURE_RESULT, *PMEASURE_RESULT;

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

#define NA_INTEGER				-(INT_MAX)

typedef struct _IQANALYZE_SETTING
{
	//802.11b
	int wifi_11b_eq_taps;
	int wifi_11b_DCremove11b_flag;
	int wifi_11b_method_11b;
	//802.11ag
	int wifi_11ag_ph_corr_mode;
	int wifi_11ag_ch_estimate;
	int wifi_11ag_sym_tim_corr;
	int wifi_11ag_freq_sync;
	int wifi_11ag_ampl_track;
	//802.11n
	TCHAR wifi_11n_type[STRING_LENS_16];
	TCHAR wifi_11n_mode[STRING_LENS_16];
	int wifi_11n_enablePhaseCorr;
	int wifi_11n_enableSymTimingCorr;
	int wifi_11n_enableAmplitudeTracking;
	int wifi_11n_decodePSDU;
	int wifi_11n_enableFullPacketChannelEst;
	int wifi_11n_packetFormat;
	int wifi_11n_frequencyCorr;
	//802.11ac
	TCHAR wifi_11ac_mode[STRING_LENS_16];
	int wifi_11ac_enablePhaseCorr;
	int wifi_11ac_enableSymTimingCorr;
	int wifi_11ac_enableAmplitudeTracking;
	int wifi_11ac_decodePSDU;
	int wifi_11ac_enableFullPacketChannelEst;
	int wifi_11ac_frequencyCorr;
	int wifi_11ac_packetFormat;

	inline _IQANALYZE_SETTING()
	{
		wifi_11b_eq_taps = 1;						//IQV_EQ_OFF = 1, IQV_EQ_5_TAPS = 5, IQV_EQ_7_TAPS = 7, IQV_EQ_9_TAPS = 9 
		wifi_11b_DCremove11b_flag = 0;				//IQV_DC_REMOVAL_OFF = 0, IQV_DC_REMOVAL_ON  = 1
		wifi_11b_method_11b = 1;					//IQV_11B_STANDARD_TX_ACC  = 1, IQV_11B_RMS_ERROR_VECTOR = 2
		//11ag
		wifi_11ag_ph_corr_mode = 2;					//IQV_PH_CORR_OFF = 1, IQV_PH_CORR_SYM_BY_SYM = 2, IQV_PH_CORR_MOVING_AVG = 3   
		wifi_11ag_ch_estimate = 1;					//IQV_CH_EST_RAW =1, IQV_CH_EST_RAW_LONG = IQV_CH_EST_RAW, IQV_CH_EST_2ND_ORDER	= 2, IQV_CH_EST_RAW_FULL = 3 
		wifi_11ag_sym_tim_corr = 2;					//IQV_SYM_TIM_OF = 1, IQV_SYM_TIM_ON = 2
		wifi_11ag_freq_sync = 2;					//IQV_FREQ_SYNC_SHORT_TRAIN = 1, IQV_FREQ_SYNC_LONG_TRAIN  = 2, IQV_FREQ_SYNC_FULL_PACKET = 3
		wifi_11ag_ampl_track = 1;					//IQV_AMPL_TRACK_OFF = 1, IQV_AMPL_TRACK_ON	= 2
		//11n
		strcpy(wifi_11n_type, _T("EWC"));
		strcpy(wifi_11n_mode, _T("nxn"));
		wifi_11n_enablePhaseCorr = 1;				// 1: enabled, 0: disabled
		wifi_11n_enableSymTimingCorr = 1;			// 1: enabled, 0: disabled
		wifi_11n_enableAmplitudeTracking = 0;		// 1: enabled, 0: disabled
		wifi_11n_decodePSDU = 1;					// 1: enabled, 0: disabled
		wifi_11n_enableFullPacketChannelEst = 0;	// 1: enabled, 0: disabled
		wifi_11n_packetFormat = 0;					// 0: auto-detect mode (Default), 1: mixed format, 2: greenfield format
		wifi_11n_frequencyCorr = 2;					// 2: Long Training Field, 3: Full Packet, 4: LTF+SIG Field
		//11ac
		strcpy(wifi_11ac_mode, _T("nxn"));
		wifi_11ac_enablePhaseCorr = 1;				// 0: Disable, 1: Enable (Default)
		wifi_11ac_enableSymTimingCorr = 1;			// 0: Disable, 1: Enable (Default)
		wifi_11ac_enableAmplitudeTracking = 0;		// 0: Disable (Default), 1: Enable
		wifi_11ac_decodePSDU = 0;					// 0: Disable (Default), 1: Enable
		wifi_11ac_enableFullPacketChannelEst = 0;	// 0: Disable (Default), 1: Enable
		wifi_11ac_frequencyCorr = 2;				// 1:Frequency correction on short legacy training fields
		// 2:Frequency correction on long legacy training fields
		// 3:Frequency correction based on whole packet
		// 4:Frequency correction on signal fields (legacy and HT) in addition to long training fields
		wifi_11ac_packetFormat = 4;					// 0: auto-detect mode (Default), 1: mixed format (11n), 2: greenfield format (11n), 3: legacy format (11a/g), 4: VHT format (11ac)
	}
}IQANALYZE_SETTING, *PIQANALYZE_SETTING;

typedef struct _IQVSA_SETTINGS
{
	ANALYZE_TYPE AnalyzeType;
	double Frequency;
	BAND_WIDTH BandWidth;
	double Pathloss;
	double AllPathloss[4];
	double rfAmplDb;
	double triggerLevelDb;
	double triggerPreTime;
	double samplingTimeSecs;
	int triggerType;
	int ht40Mode;	
	TCHAR wifi_mode[STRING_LENS_16];
	TCHAR szRefFile[MAX_PATH];
}IQVSA_SETTINGS, *PIQVSA_SETTINGS;

typedef struct _IQVSG_SETTINGS
{
	double Frequency;
	double rfPowerLeveldBm;
	TCHAR szModFile[MAX_PATH];
	int frameCnt;
}IQVSG_SETTINGS, *PIQVSG_SETTINGS;

#define LP_BUFFER_SIZE	8192 /*Reference from LPIQ.h*/
typedef struct _SPECTRUM_MASK
{
	int fftLength;
	double freqsHz[(LP_BUFFER_SIZE	*8)*sizeof(double)];	
	double tracedbm[(LP_BUFFER_SIZE	*8)*sizeof(double)];
	double *SpectrumMask;
}SPECTRUM_MASK, *PSPECTRUM_MASK;

#endif