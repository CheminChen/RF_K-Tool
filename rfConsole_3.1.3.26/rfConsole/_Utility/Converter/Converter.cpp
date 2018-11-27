#include "StdAfx.h"
#include "Converter.h"


CConverter::CConverter(void)
{
}

CConverter::~CConverter(void)
{
}

LPSTR CConverter::GetModFile(RF_TYPE rfType, ANALYZE_TYPE AnalyzeType, DATA_RATE DataRate)
{
	CString strFile = _T(""), strType = _T(""), strRate = _T(""), strBW = _T("");

	if(AnalyzeType == STANDARD_802_11_B)
	{
		if(DataRate == DATARATE_DSSS_1) return _T("WiFi_DSSS-1");
		else if(DataRate == DATARATE_DSSS_2) return _T("DSSS_2");
		else if(DataRate == DATARATE_CCK_5_5) return _T("CCK_5_5");
		else if(DataRate == DATARATE_CCK_11) return _T("CCK_11");
	}

	return _T("");
}

LPSTR CConverter::RfType2String(RF_TYPE rfType)
{
	if(rfType == RF_NI) return _T("NI");
	else if(rfType == RF_ITEST) return _T("ITEST");
	else if(rfType == RF_IQXEL_80) return _T("IQXEL80");
	else if(rfType == RF_IQXEL_M8) return _T("IQXEL-M8");
	else if(rfType == RF_IQXEL_M2W) return _T("IQXEL-M2W");

	return _T("");
}

LPSTR CConverter::AnalyzeType2String(ANALYZE_TYPE AnalyzeType)
{
	if(AnalyzeType == STANDARD_802_11_B) return _T("STANDARD_802_11_B");
	else if(AnalyzeType == STANDARD_802_11_AG) return _T("STANDARD_802_11_AG");
	else if(AnalyzeType == STANDARD_802_11_N) return _T("STANDARD_802_11_N");
	else if(AnalyzeType == STANDARD_802_11_AC) return _T("STANDARD_802_11_AC");
	else if(AnalyzeType == STANDARD_BLUETOOTH) return _T("STANDARD_BLUETOOTH");
	else if(AnalyzeType == STANDARD_ZIGBEE) return _T("STANDARD_ZIGBEE");

	return _T("");
}

LPSTR CConverter::Antenna2String(ANT_SELECTION AntSel)
{
	if(AntSel == TX_CHAIN_A) return _T("TX1");
	else if(AntSel == TX_CHAIN_B) return _T("TX2");
	else if(AntSel == TX_CHAIN_C) return _T("TX3");
	else if(AntSel == TX_CHAIN_D) return _T("TX4");

	else if(AntSel == TX_CHAIN_AB) return _T("TX1-TX2");
	else if(AntSel == TX_CHAIN_AC) return _T("TX1-TX3");
	else if(AntSel == TX_CHAIN_BC) return _T("TX2-TX3");

	else if(AntSel == TX_CHAIN_ABC) return _T("TX1-TX2-TX3");
	else if(AntSel == TX_CHAIN_ALL) return _T("TX1-TX2-TX3-TX4");

	return _T("");
}

LPSTR CConverter::Bandwidth2String(BAND_WIDTH BandWidth)
{
	if(BandWidth == BW_20MHZ) return _T("BW-20");
	else if(BandWidth == BW_40MHZ) return _T("BW-40");
	else if(BandWidth == BW_80MHZ) return _T("BW-80");
	else if(BandWidth == BW_160MHZ) return _T("BW-160");
	else if(BandWidth == BW_80_80MHZ) return _T("BW_80_80");

	return _T("");
}

LPSTR CConverter::DataRate2String(DATA_RATE DataRate)
{
	if(DataRate == DATARATE_DSSS_1) return _T("DSSS_1");
	else if(DataRate == DATARATE_DSSS_2) return _T("DSSS_2");
	else if(DataRate == DATARATE_CCK_5_5) return _T("CCK_5_5");
	else if(DataRate == DATARATE_CCK_11) return _T("CCK_11");

	else if(DataRate == DATARATE_OFDM_6) return _T("OFDM_6");
	else if(DataRate == DATARATE_OFDM_9) return _T("OFDM_9");
	else if(DataRate == DATARATE_OFDM_12) return _T("OFDM_12");
	else if(DataRate == DATARATE_OFDM_18) return _T("OFDM_18");
	else if(DataRate == DATARATE_OFDM_24) return _T("OFDM_24");
	else if(DataRate == DATARATE_OFDM_36) return _T("OFDM_36");
	else if(DataRate == DATARATE_OFDM_48) return _T("OFDM_48");
	else if(DataRate == DATARATE_OFDM_54) return _T("OFDM_54");

	else if(DataRate == DATARATE_MCS0) return _T("MCS0");
	else if(DataRate == DATARATE_MCS1) return _T("MCS1");
	else if(DataRate == DATARATE_MCS2) return _T("MCS2");
	else if(DataRate == DATARATE_MCS3) return _T("MCS3");
	else if(DataRate == DATARATE_MCS4) return _T("MCS4");
	else if(DataRate == DATARATE_MCS5) return _T("MCS5");
	else if(DataRate == DATARATE_MCS6) return _T("MCS6");
	else if(DataRate == DATARATE_MCS7) return _T("MCS7");
	else if(DataRate == DATARATE_MCS8) return _T("MCS8");
	else if(DataRate == DATARATE_MCS9) return _T("MCS9");
	else if(DataRate == DATARATE_MCS10) return _T("MCS10");
	else if(DataRate == DATARATE_MCS11) return _T("MCS11");
	else if(DataRate == DATARATE_MCS12) return _T("MCS12");
	else if(DataRate == DATARATE_MCS13) return _T("MCS13");
	else if(DataRate == DATARATE_MCS14) return _T("MCS14");
	else if(DataRate == DATARATE_MCS15) return _T("MCS15");
	else if(DataRate == DATARATE_MCS16) return _T("MCS16");
	else if(DataRate == DATARATE_MCS17) return _T("MCS17");
	else if(DataRate == DATARATE_MCS18) return _T("MCS18");
	else if(DataRate == DATARATE_MCS19) return _T("MCS19");
	else if(DataRate == DATARATE_MCS20) return _T("MCS20");
	else if(DataRate == DATARATE_MCS21) return _T("MCS21");
	else if(DataRate == DATARATE_MCS22) return _T("MCS22");
	else if(DataRate == DATARATE_MCS23) return _T("MCS23");
	else if(DataRate == DATARATE_MCS24) return _T("MCS24");
	else if(DataRate == DATARATE_MCS25) return _T("MCS25");
	else if(DataRate == DATARATE_MCS26) return _T("MCS26");
	else if(DataRate == DATARATE_MCS27) return _T("MCS27");
	else if(DataRate == DATARATE_MCS28) return _T("MCS28");
	else if(DataRate == DATARATE_MCS29) return _T("MCS29");
	else if(DataRate == DATARATE_MCS30) return _T("MCS30");
	else if(DataRate == DATARATE_MCS31) return _T("MCS31");

	else if(DataRate == DATARATE_BT_1DH1) return _T("BT_1DH1");
	else if(DataRate == DATARATE_BT_1DH3) return _T("BT_1DH3");
	else if(DataRate == DATARATE_BT_1DH5) return _T("BT_1DH5");

	else if(DataRate == DATARATE_BT_2DH1) return _T("BT_2DH1");
	else if(DataRate == DATARATE_BT_2DH3) return _T("BT_2DH3");
	else if(DataRate == DATARATE_BT_2DH5) return _T("BT_2DH5");
	else if(DataRate == DATARATE_BT_3DH1) return _T("BT_3DH1");
	else if(DataRate == DATARATE_BT_3DH3) return _T("BT_3DH3");
	else if(DataRate == DATARATE_BT_3DH5) return _T("BT_3DH5");

	return _T("");
}

int CConverter::Frequency2Channel(int Freq)
{
	int CH = -1;

	if(Freq <= 2472) CH = (Freq-2407)/5;
	else CH = ((Freq-5000)/10)*2;

	return CH;
}

double CConverter::Channel2Frequency(int CH)
{
	double Freq = -1;

	if(CH <= 14) Freq = (CH*5)+2407;
	else Freq = ((CH/2)*10)+5000;

	return (Freq*1000000);
}

double CConverter::PeakToAvgPower(MODULATION_TYPE Modulation)
{
	if(Modulation == MODULATION_CW) return 1;
	else if(Modulation == MODULATION_DSSS) return 4;
	else return 10;
}

double CConverter::Modulation2SamplingTime(MODULATION_TYPE MType)
{
	if(MType == MODULATION_CW) return 50e-6;
	else if(MType == MODULATION_DSSS) return 1000e-6;		//Gemtek: 0.0014
	else if(MType == MODULATION_OFDM) return 500e-6;
}

ANALYZE_TYPE CConverter::DataRate2AnalyzeType(DATA_RATE DataRate)
{
	if((DataRate == DATARATE_DSSS_1) || (DataRate == DATARATE_DSSS_2) || (DataRate == DATARATE_CCK_5_5) || (DataRate == DATARATE_CCK_11))			
		return STANDARD_802_11_B;	
	else if((DataRate == DATARATE_OFDM_6) || (DataRate == DATARATE_OFDM_9) || (DataRate == DATARATE_OFDM_12) || (DataRate == DATARATE_OFDM_18) ||
		(DataRate == DATARATE_OFDM_24) || (DataRate == DATARATE_OFDM_36) || (DataRate == DATARATE_OFDM_48) || (DataRate == DATARATE_OFDM_54))
		return STANDARD_802_11_AG;

}

IQV_CAPTURE_TYPE_ENUM CConverter::Bandwidth2CaptureType(BAND_WIDTH BandWidth)
{
	if(BandWidth == BW_40MHZ) return IQV_HT_40_CAPTURE_TYPE;
	else if(BandWidth == BW_80MHZ) return IQV_VHT_80_WIDE_BAND_CAPTURE_TYPE;
	else return IQV_NORMAL_CAPTURE_TYPE;
}

MODULATION_TYPE CConverter::DataRate2Modulation(DATA_RATE DataRate)
{
	if((DataRate == DATARATE_DSSS_1) || (DataRate == DATARATE_DSSS_2) || (DataRate == DATARATE_CCK_5_5) || (DataRate == DATARATE_CCK_11))			
		return MODULATION_DSSS;	
	/*
	else if((DataRate == DATARATE_CCK_5_5) || (DataRate == DATARATE_CCK_11))	
		return 3.0;			*/
	else if((DataRate == DATARATE_OFDM_6) || (DataRate == DATARATE_OFDM_9) || (DataRate == DATARATE_OFDM_12) || (DataRate == DATARATE_OFDM_18) ||
		(DataRate == DATARATE_OFDM_24) || (DataRate == DATARATE_OFDM_36) || (DataRate == DATARATE_OFDM_48) || (DataRate == DATARATE_OFDM_54) ||
		
		(DataRate == DATARATE_MCS0) || (DataRate == DATARATE_MCS1) || (DataRate == DATARATE_MCS2) || (DataRate == DATARATE_MCS3) ||
		(DataRate == DATARATE_MCS4) || (DataRate == DATARATE_MCS5) || (DataRate == DATARATE_MCS6) || (DataRate == DATARATE_MCS7) ||
		(DataRate == DATARATE_MCS8) || (DataRate == DATARATE_MCS9) || (DataRate == DATARATE_MCS10) || (DataRate == DATARATE_MCS11) ||
		(DataRate == DATARATE_MCS12) || (DataRate == DATARATE_MCS13) || (DataRate == DATARATE_MCS14) || (DataRate == DATARATE_MCS15) ||
		(DataRate == DATARATE_MCS16) || (DataRate == DATARATE_MCS17) || (DataRate == DATARATE_MCS18) || (DataRate == DATARATE_MCS19) ||
		(DataRate == DATARATE_MCS20) || (DataRate == DATARATE_MCS21) || (DataRate == DATARATE_MCS22) || (DataRate == DATARATE_MCS23) ||
		(DataRate == DATARATE_MCS24) || (DataRate == DATARATE_MCS25) || (DataRate == DATARATE_MCS26) || (DataRate == DATARATE_MCS27) ||
		(DataRate == DATARATE_MCS28) || (DataRate == DATARATE_MCS29) || (DataRate == DATARATE_MCS30) || (DataRate == DATARATE_MCS31))
		return MODULATION_OFDM;
	else 
		return MODULATION_CW;		
}