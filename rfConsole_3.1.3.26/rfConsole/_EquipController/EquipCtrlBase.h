#pragma once

#include "..\_Utility\Logger\Logger.h"
#include "..\..\..\Lib\_Import\IQmeasure_3.1.3.26\Include\IQmeasure.h"

class EquipCtrlbase
{
public:
	virtual void InitializeEquip(CLogger *pLogger) = 0;
	virtual void TerminateEquip() = 0;

	virtual bool ConnectDevice(void *dataStruct, LPSTR szRespMsg, int nLens=STRING_LENS_4096) = 0;

	virtual bool SetupVSG(void *dataStruct, LPSTR szRespMsg, int nLens=STRING_LENS_4096) = 0;
	virtual bool SetupVSA(void *dataStruct, LPSTR szRespMsg, int nLens=STRING_LENS_4096) = 0;

	virtual bool StartMeasure(void *dataStruct, LPSTR szRespMsg, int nLens=STRING_LENS_4096) = 0;
	virtual bool StartGenerate(int frameCnt, LPSTR szRespMsg, int nLens=STRING_LENS_4096) = 0;

	virtual bool DisconnectDevice(LPSTR szRespMsg, int nLens=STRING_LENS_4096) = 0;

};