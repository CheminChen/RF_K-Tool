#pragma once
#include "..\..\..\..\Lib\rfDefine.h"
#include "..\CommFunc\CommFunc.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")


class myPing
{
public:
	myPing(void);
	~myPing(void);

public:
	bool PingUntilAilve(LPSTR lpIP, int nPingTimeout, int nSucessCnt=3, int nPingInterval=500);
	bool PingUntilDead(LPSTR lpIP, int nPingTimeout, int nSucessCnt=3);

private:
	void OutputTraceLog(TCHAR *format, ...);
};

