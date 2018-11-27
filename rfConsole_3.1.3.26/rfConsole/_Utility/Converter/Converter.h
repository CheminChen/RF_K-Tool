#pragma once

#include "..\..\..\..\Lib\rfDefine.h"

class CConverter
{
public:
	CConverter(void);
	~CConverter(void);

public:
	static double Channel2Frequency(int CH);
	static int Frequency2Channel(int Freq);

	static LPSTR DataRate2String(DATA_RATE DataRate);
	static LPSTR Bandwidth2String(BAND_WIDTH BandWidth);
	static LPSTR Antenna2String(ANT_SELECTION AntSel);
	static LPSTR AnalyzeType2String(ANALYZE_TYPE AnalyzeType);
	static LPSTR RfType2String(RF_TYPE rfType);

	static double PeakToAvgPower(MODULATION_TYPE Modulation);
	static MODULATION_TYPE DataRate2Modulation(DATA_RATE DataRate);
	static double Modulation2SamplingTime(MODULATION_TYPE MType);		/*???, Needs to confirm*/
	static ANALYZE_TYPE DataRate2AnalyzeType(DATA_RATE DataRate);
	static IQV_CAPTURE_TYPE_ENUM Bandwidth2CaptureType(BAND_WIDTH BandWidth);

	static LPSTR GetModFile(RF_TYPE rfType, ANALYZE_TYPE AnalyzeType, DATA_RATE DataRate);
};

