#pragma once
#include "..\Logger\Logger.h"
#include "..\CommFunc\CommFunc.h"

#define SOCKET_BUFFER_SIZE				65535
#define CLOSE_SOCKET_TIMEOUT				2000
#define MAX_CMD_LIMIT					256

class mySocket
{
public:
	mySocket(void);
	~mySocket(void);

public:
	bool InitializeSocket(CLogger *pLogger=NULL);
		
	bool OpenTelnet(TCHAR* szIPAddr, TCHAR* szLoginPrompt, TCHAR* szLoginID, TCHAR* szPwdPrompt, TCHAR* szLoginPwd, TCHAR *szShellPrompt, LPSTR szRespMsg, int nLens=STRING_LENS_4096);
	void CloseTelnet();
	bool SendTelnetCmd(TCHAR* szCmd, TCHAR* szChkString, int nTimeout, LPSTR szRespMsg, int nLens, bool bChkResponse=true);

	bool OpenSocket(TCHAR* szIPAddr, int nPort);
	void CloseSocket();
	bool SendSocketCmd(TCHAR* szCmd, int nTimeout, LPSTR szRespMsg, int nLens);
	

private:
	static UINT TelnetRecvThread(LPVOID pParam);
	static UINT SocketRecvThread(LPVOID pParam);

	void OutputTraceLog(TCHAR *format, ...);

	CLogger					*m_pLogger;
	CRITICAL_SECTION		m_cs;  

	//Thread
	HANDLE					m_hEvtExitThread, m_hEvtLogin, m_hEvtPwd, m_hEvtRecv;
	bool					m_bExitThread;

	//Socket
	SOCKET					m_sConnect;
	TCHAR					m_SendBuffer[SOCKET_BUFFER_SIZE], m_RecvBuffer[SOCKET_BUFFER_SIZE];
	TCHAR					m_CheckString[MAX_CMD_LIMIT];
	TCHAR					m_FilterCmd[MAX_CMD_LIMIT];

	//Telnet
	TCHAR					m_LoginPrompt[STRING_LENS_32], m_PwdPrompt[STRING_LENS_32];
	//bool					m_IgnoreResponse;

	bool					m_bLengthFlag;
	//Debug
	bool					m_bDebug;
};

