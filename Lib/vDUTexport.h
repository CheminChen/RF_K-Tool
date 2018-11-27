#ifndef _VDUT_DEFINE_H__
#define _VDUT_DEFINE_H__

#include "rfDefine.h"

#if 0
extern "C" { __declspec(dllexport) void InitializeDUT(); }
extern "C" { __declspec(dllexport) void TerminateDUT(); }

extern "C" { __declspec(dllexport) bool vDUT_Init(DUT_SETTINGS *pDUTSettings, LPSTR szRespMsg, int nLens); }
extern "C" { __declspec(dllexport) bool vDUT_WaitReady(int nWaitTimems, LPSTR szRespMsg, int nLens); }
extern "C" { __declspec(dllexport) bool vDUT_Remove(LPSTR szRespMsg, int nLens); }

extern "C" { __declspec(dllexport) bool vDUT_SetTx(void *dataStruct, LPSTR szRespMsg, int nLens); }
extern "C" { __declspec(dllexport) bool vDUT_SetRx(void *dataStruct, LPSTR szRespMsg, int nLens); }

extern "C" { __declspec(dllexport) bool vDUT_Start(RF_MODE rfMode, void *dataStruct, LPSTR szRespMsg, int nLens); }
extern "C" { __declspec(dllexport) bool vDUT_Stop(RF_MODE rfMode, void *dataStruct, LPSTR szRespMsg, int nLens); }
extern "C" { __declspec(dllexport) bool vDUT_ResetRx(LPSTR szRespMsg, int nLens); }

extern "C" { __declspec(dllexport) bool vDUT_Commit(LPSTR szRespMsg, int nLens); }

#endif

typedef bool (CALLBACK *cbOpenTelnet)(TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR*, LPSTR, int);
typedef void (CALLBACK *cbCloseTelnet)();
typedef bool (CALLBACK *cbSendTelnetCmd)(TCHAR*, TCHAR*, int, LPSTR, int, bool);
typedef bool (CALLBACK *cbSetupVSA)(RF_SETTINGS*, LPSTR, int);
typedef bool (CALLBACK *cbSetupVSG)(RF_SETTINGS*, LPSTR, int);
typedef bool (CALLBACK *cbStartMeasure)(MEASURE_RESULT *, LPSTR, int);
typedef bool (CALLBACK *cbStartGenerate)(int, LPSTR, int);
typedef bool (CALLBACK *cbGetTesterLock)(LPSTR, int);
typedef bool (CALLBACK *cbReleaseTesterLock)(LPSTR, int);
typedef void (CALLBACK *cbOutputLog)(LOGTYPE, TCHAR*, ...);

//extern "C" VOID PASCAL EXPORT InitializeDUT(cbSetupVSA SetupVSA, cbSetupVSG SetupVSG, cbStartMeasure StartMeasure, cbStartGenerate StartGenerate, cbOutputLog OutputLog, cbOpenTelnet OpenTelnet=NULL, cbCloseTelnet CloseTelnet=NULL, cbSendTelnetCmd SendTelnetCmd=NULL);
extern "C" VOID PASCAL EXPORT InitializeDUT(cbSetupVSA SetupVSA, cbSetupVSG SetupVSG, cbStartMeasure StartMeasure, cbStartGenerate StartGenerate, cbGetTesterLock GetTesterLock, cbReleaseTesterLock ReleaseTesterLock, cbOutputLog OutputLog, cbOpenTelnet OpenTelnet=NULL, cbCloseTelnet CloseTelnet=NULL, cbSendTelnetCmd SendTelnetCmd=NULL);
extern "C" VOID PASCAL EXPORT TerminateDUT();

extern "C" BOOL PASCAL EXPORT vDUT_Init(DUT_SETTINGS *pDUTSettings, LPSTR szRespMsg, int nLens);
extern "C" BOOL PASCAL EXPORT vDUT_WaitReady(int nWaitTimems, LPSTR szRespMsg, int nLens);
extern "C" BOOL PASCAL EXPORT vDUT_Remove(LPSTR szRespMsg, int nLens);

extern "C" BOOL PASCAL EXPORT vDUT_SetTx(void *dataStruct, LPSTR szRespMsg, int nLens);
extern "C" BOOL PASCAL EXPORT vDUT_SetRx(void *dataStruct, LPSTR szRespMsg, int nLens);

extern "C" BOOL PASCAL EXPORT vDUT_Start(RF_MODE rfMode, void *dataStruct, LPSTR szRespMsg, int nLens);
extern "C" BOOL PASCAL EXPORT vDUT_Stop(RF_MODE rfMode, void *dataStruct, LPSTR szRespMsg, int nLens);
extern "C" BOOL PASCAL EXPORT vDUT_ResetRx(LPSTR szRespMsg, int nLens);

extern "C" BOOL PASCAL EXPORT vDUT_Calibration(CAL_TYPE calType, void *dataStruct, LPSTR szRespMsg, int nLens);

extern "C" BOOL PASCAL EXPORT vDUT_Commit(LPSTR szRespMsg, int nLens);



#endif