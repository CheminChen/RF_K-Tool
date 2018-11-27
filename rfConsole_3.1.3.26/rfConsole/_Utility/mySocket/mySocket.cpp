#include "StdAfx.h"
#include "mySocket.h"


mySocket::mySocket(void)
{
	m_pLogger = NULL;
	m_bDebug = m_bLengthFlag = false;

	m_hEvtExitThread = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEvtLogin = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEvtPwd = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEvtRecv = CreateEvent(NULL, TRUE, FALSE, NULL);

	memset(&m_SendBuffer, 0, sizeof(m_SendBuffer));
	memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));
	//memset(&m_FilterCmd, 0, sizeof(m_FilterCmd));
	//memset(&m_CheckString, 0, sizeof(m_CheckString));
	memset(&m_LoginPrompt, 0, STRING_LENS_32);
	memset(&m_PwdPrompt, 0, STRING_LENS_32);	

	strcpy_s(m_LoginPrompt, sizeof(m_LoginPrompt), _T("login:"));
	strcpy_s(m_PwdPrompt, sizeof(m_PwdPrompt), _T("password:"));
	strcpy_s(m_FilterCmd, sizeof(m_FilterCmd), _T("#"));
	strcpy_s(m_CheckString, sizeof(m_CheckString), _T("#"));
}

mySocket::~mySocket(void)
{
}

void mySocket::OutputTraceLog(TCHAR *format, ...)
{
#if (_DEBUG)
	TCHAR *pBuffer = new TCHAR[SOCKET_BUFFER_SIZE];
	va_list marker;

	memset(pBuffer, 0, SOCKET_BUFFER_SIZE);
	va_start(marker, format);
	vsprintf_s(pBuffer, SOCKET_BUFFER_SIZE, format, marker);
	va_end(marker);

	OutputDebugString(pBuffer);	
	DEL_ARRAY(pBuffer)
#endif
}

bool mySocket::InitializeSocket(CLogger *pLogger)
{
	if(pLogger) m_pLogger = pLogger;

	//Initialize Settings
	m_bExitThread = true;
	InitializeCriticalSection(&m_cs);	

	//Initialize Socket
	WSAData wsaData;
	WORD version = MAKEWORD(2, 2); 
	return (WSAStartup(MAKEWORD(2,2), &wsaData) == 0);
}

#define	MAX_STACK_SIZE	655350
bool mySocket::OpenTelnet(TCHAR* szIPAddr, TCHAR* szLoginPrompt, TCHAR* szLoginID, TCHAR* szPwdPrompt, TCHAR* szLoginPwd, TCHAR *szShellPrompt, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	SOCKADDR_IN addr;
	int addlen = sizeof(addr), nRtn;
	unsigned long non_blocking = 1; 

	if(szLoginPrompt) strcpy_s(m_LoginPrompt, STRING_LENS_32, szLoginPrompt);
	if(szPwdPrompt) strcpy_s(m_PwdPrompt, STRING_LENS_32, szPwdPrompt);

	//Create Socket
	if((m_sConnect = socket(AF_INET, SOCK_STREAM, NULL)) != INVALID_SOCKET)
	{
		addr.sin_addr.s_addr = inet_addr(szIPAddr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(23);
		memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));

		nRtn = connect(m_sConnect, (SOCKADDR*)&addr, sizeof(addr));		
		if(!(nRtn != SOCKET_ERROR)) 
		{
			OutputTraceLog(_T("call connect failed, Server: %s@%d, errNo: %d"), szIPAddr, 23, WSAGetLastError());
			goto go_End;
		}
		ioctlsocket(m_sConnect, FIONBIO, &non_blocking); 

		//Create Thread
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		if((m_bExitThread) && (AfxBeginThread(TelnetRecvThread, this, THREAD_PRIORITY_NORMAL, MAX_STACK_SIZE, 0, &sa) == NULL)) goto go_End;

		/*Need to login*/
		if(strcmp(szLoginID, _T("")) != 0) 
		{
			/*Send the hello to DUT*/
			sprintf_s(m_SendBuffer, sizeof(m_SendBuffer), "\r\n");
			memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));
			send(m_sConnect, m_SendBuffer, strlen(m_SendBuffer), 0);

			//Input ID for login
			if(WaitForSingleObject(m_hEvtLogin, 5*1000) == WAIT_TIMEOUT)
			{
				if(szRespMsg) strcpy_s(szRespMsg, nLens, _T("Fail to wait login Prompt"));
				OutputTraceLog(_T("Timeout, Fail to wait login Prompt"));

				goto go_End;	
			}

			OutputTraceLog("\r\nLogin process, szLoginID: %s", szLoginID);
			if(strstr(m_RecvBuffer, szLoginPrompt))
			{
				sprintf_s(m_SendBuffer, sizeof(m_SendBuffer), "%s\r\n", szLoginID);
				memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));
				send(m_sConnect, m_SendBuffer, strlen(m_SendBuffer), 0);

			}else
			{
				if(szRespMsg) sprintf_s(szRespMsg, nLens, _T("Fail to find the login prompt, Recv: %s"), m_RecvBuffer);
				OutputTraceLog(_T("Fail to find the login Prompt, Recv: %s"), m_RecvBuffer);

				goto go_End;	
			}

			//Input Password for login
			if(WaitForSingleObject(m_hEvtPwd, 5*1000) == WAIT_TIMEOUT) goto go_End;

			OutputTraceLog("\r\nPassword process, szLoginPwd: %s", szLoginPwd);
			if(strstr(m_RecvBuffer, szPwdPrompt))
			{
				sprintf_s(m_SendBuffer, sizeof(m_SendBuffer), "%s\r\n", (strcmp(szLoginPwd, _T("")) != 0)?szLoginPwd:_T(""));
				memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));
				send(m_sConnect, m_SendBuffer, strlen(m_SendBuffer), 0);

			}else
			{
				if(szRespMsg) sprintf_s(szRespMsg, nLens, _T("Fail to find the password prompt, Recv: %s"), m_RecvBuffer);
				OutputTraceLog(_T("Fail to find the password prompt, Recv: %s"), m_RecvBuffer);

				goto go_End;	
			}		
		}

		//Send '\r\n'
		strcpy_s(m_FilterCmd, sizeof(m_FilterCmd), _T("\r\n"));
		OutputTraceLog("\r\nAfter login success, checkString: %s, bRtn: %d", m_CheckString, bRtn);
		send(m_sConnect, m_FilterCmd, strlen(m_FilterCmd), 0);
		if(WaitForSingleObject(m_hEvtRecv, 5*1000) == WAIT_TIMEOUT) goto go_End;		
		ResetEvent(m_hEvtRecv);
		
		if(szRespMsg)
		{
			if(strlen(m_RecvBuffer) > nLens) strcpy_s(szRespMsg, nLens, m_RecvBuffer);
			else OutputTraceLog(m_RecvBuffer);
		}
		bRtn = true;
	}

go_End:
	return bRtn;
}

void mySocket::CloseTelnet()
{
	CloseSocket();
}

bool mySocket::SendTelnetCmd(TCHAR* szCmd, TCHAR* szChkString, int nTimeout, LPSTR szRespMsg, int nLens, bool bChkResponse)
{
	bool bRtn = false;
	int nRtn;
	TCHAR *pRespStart = NULL;

	memset(&m_SendBuffer, 0, sizeof(m_SendBuffer));
	if(szChkString) strcpy_s(m_CheckString, sizeof(m_CheckString), szChkString);

	/*Record command filter*/
	if(strlen(szCmd) > MAX_CMD_LIMIT) strncpy_s(m_FilterCmd, sizeof(m_FilterCmd), szCmd, MAX_CMD_LIMIT-1);
	else strcpy_s(m_FilterCmd, sizeof(m_FilterCmd), szCmd);
	OutputTraceLog(_T("\r\nTelnet Send : >%s<"), m_FilterCmd);

	/*Start sending*/
	sprintf_s(m_SendBuffer, sizeof(m_SendBuffer), "%s\r\n", szCmd);	
	memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));
	ResetEvent(m_hEvtRecv);
	if(!(nRtn = send(m_sConnect, m_SendBuffer, strlen(m_SendBuffer), 0))) goto go_End;
	if(bChkResponse && (WaitForSingleObject(m_hEvtRecv, nTimeout*1000) == WAIT_TIMEOUT))
	{
		sprintf_s(szRespMsg, nLens, _T("Fail to find the match string, Recv: %s"), m_RecvBuffer);
		goto go_End;
	}

	bRtn = true;
go_End:
	if(bChkResponse)
	{
		if(m_pLogger) m_pLogger->OutputLog(TYPE_DETAILS, COLOR_WHITE, _T("> Send (%d): %s\r\n> %s"), nRtn, szCmd, m_RecvBuffer);

		if(szRespMsg && strlen(m_RecvBuffer) < nLens)
		{
			strcpy_s(szRespMsg, nLens, m_RecvBuffer);
		}
		else
		{
			bRtn = false;
			sprintf_s(szRespMsg, nLens, _T("Buffer overflow over the length: %d"), strlen(m_RecvBuffer));
			OutputTraceLog(_T("Buffer overflow (over the length: %d), Recv: %s"), nLens, m_RecvBuffer);
		}
	}else 
	{
		if(m_pLogger) m_pLogger->OutputLog(TYPE_DETAILS, COLOR_WHITE, _T("> Send (%d): %s"), nRtn, szCmd);
	}
	ResetEvent(m_hEvtRecv);
	return bRtn;
}

UINT mySocket::TelnetRecvThread(LPVOID pParam)
{
	mySocket *pMain = (mySocket*)pParam;
	int nResult, nLen, nCmdLens, nBLLens, nCmdIdx;
	TCHAR szTmp[STRING_LENS_4096] = {'\0'}, szLoopBuffer[SOCKET_BUFFER_SIZE] = {'\0'}, *pString = NULL, *pBreakLine;
	fd_set fdRead;
	struct timeval TimeVal;
	TimeVal.tv_sec = 1;
	TimeVal.tv_usec = 1000;	

	pMain->m_bExitThread = false;
	while(!pMain->m_bExitThread)
	{
		FD_ZERO(&fdRead);
		FD_SET(pMain->m_sConnect, &fdRead);

		nResult = select(pMain->m_sConnect+1, &fdRead, 0, 0, &TimeVal);
		if(nResult == -1) break;					//Error
		else if(nResult == 0) continue;				//Timeout
		else
		{
			if(FD_ISSET(pMain->m_sConnect, &fdRead))
			{
				memset(&szTmp, 0, STRING_LENS_4096);

				if((nLen = recv(pMain->m_sConnect, szTmp, STRING_LENS_4096, 0)) != 0)
				{
					strcat_s(szLoopBuffer, SOCKET_BUFFER_SIZE, szTmp);					
				}	
				
				if(strstr(szLoopBuffer, pMain->m_LoginPrompt)) 
				{
					EnterCriticalSection(&pMain->m_cs);
					strcpy_s(pMain->m_RecvBuffer, SOCKET_BUFFER_SIZE, szLoopBuffer);
					Sleep(1);
					LeaveCriticalSection(&pMain->m_cs);

					SetEvent(pMain->m_hEvtLogin);
					memset(&szTmp, 0, STRING_LENS_4096);
					memset(&szLoopBuffer, 0, SOCKET_BUFFER_SIZE);
				}
				if(strstr(szLoopBuffer, pMain->m_PwdPrompt)) 
				{
					EnterCriticalSection(&pMain->m_cs);
					strcpy_s(pMain->m_RecvBuffer, SOCKET_BUFFER_SIZE, szLoopBuffer);
					Sleep(1);
					LeaveCriticalSection(&pMain->m_cs);

					SetEvent(pMain->m_hEvtPwd);
					memset(&szTmp, 0, STRING_LENS_4096);
					memset(&szLoopBuffer, 0, SOCKET_BUFFER_SIZE);
				}

				pBreakLine = strstr(szLoopBuffer, _T("\r\n"));
				if(pBreakLine)
				{					
					nCmdLens = strlen(pMain->m_FilterCmd);
					nBLLens = pBreakLine - &szLoopBuffer[0];

					if(nCmdLens && (nBLLens < nCmdLens))
					{
						pMain->m_FilterCmd[nBLLens-1] = '\0';
						pString = strstr(szLoopBuffer, pMain->m_FilterCmd);

						pMain->OutputTraceLog(_T("\r\nNew m_FilterCmd : >%s<"), pMain->m_FilterCmd);
					}
				}

				if((nCmdIdx = CCommFunc::FindLastOf(szLoopBuffer, pMain->m_FilterCmd)) != -1)
				{
					pMain->OutputTraceLog(_T("\r\nszLoopBuffer (%d): >%s<"), nCmdIdx, szLoopBuffer);

					if(strstr(&szLoopBuffer[nCmdIdx], pMain->m_CheckString))
					{
						EnterCriticalSection(&pMain->m_cs);
						strcpy_s(pMain->m_RecvBuffer, SOCKET_BUFFER_SIZE, &szLoopBuffer[nCmdIdx]);
						Sleep(1);
						LeaveCriticalSection(&pMain->m_cs);

						SetEvent(pMain->m_hEvtRecv);
						memset(&szTmp, 0, STRING_LENS_4096);
						memset(&szLoopBuffer, 0, SOCKET_BUFFER_SIZE);
					}					
				}

				Sleep(1);
			}	
		}
	}

go_End:
	SetEvent(pMain->m_hEvtExitThread);
	return 0;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

bool mySocket::OpenSocket(TCHAR* szIPAddr, int nPort)
{
	bool bRtn = false;
	SOCKADDR_IN addr;
	int addlen = sizeof(addr);
	unsigned long non_blocking = 1; 

	//Create Socket
	if((m_sConnect = socket(AF_INET, SOCK_STREAM, NULL)) != INVALID_SOCKET)
	{
		addr.sin_addr.s_addr = inet_addr(szIPAddr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(nPort);
		memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));
		if(!(bRtn = !connect(m_sConnect, (SOCKADDR*)&addr, sizeof(addr)))) 
		{
			OutputTraceLog(_T("call connect failed, Server: %s@%d"), szIPAddr, nPort);
			goto go_End;
		}
		ioctlsocket(m_sConnect, FIONBIO, &non_blocking); 

		//Create Thread
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		if((m_bExitThread) && (AfxBeginThread(SocketRecvThread, this, THREAD_PRIORITY_NORMAL, MAX_STACK_SIZE, 0, &sa) == NULL)) goto go_End;

		ResetEvent(m_hEvtRecv);
		bRtn = true;
	}

go_End:
	OutputTraceLog(_T("Connect to server %s"), bRtn?_T("success"):_T("fail"));
	return bRtn;
}

void mySocket::CloseSocket()
{
	if(!m_bExitThread)
	{
		ResetEvent(m_hEvtLogin);
		ResetEvent(m_hEvtPwd);
		ResetEvent(m_hEvtRecv);

		m_bExitThread = true;
		closesocket(m_sConnect);
		WaitForSingleObject(m_hEvtExitThread, CLOSE_SOCKET_TIMEOUT);	
	}	
}

bool mySocket::SendSocketCmd(TCHAR* szCmd, int nTimeout, LPSTR szRespMsg, int nLens)
{
	bool bRtn = false;
	int nRtn;
	TCHAR *pCmdLen = new TCHAR[4+1];

	memset(m_SendBuffer, 0, sizeof(m_SendBuffer));
	memset(pCmdLen, 0, sizeof(TCHAR)*(4+1));

	/*fill date struct*/
	sprintf_s(pCmdLen, sizeof(TCHAR)*(4+1), _T("%lx"), strlen(szCmd));
	for(int n=strlen(pCmdLen) ; n<4 ; n++) strcat_s(m_SendBuffer, sizeof(m_SendBuffer), _T("0"));
	strcat_s(m_SendBuffer, sizeof(m_SendBuffer), pCmdLen);
	strcat_s(m_SendBuffer, sizeof(m_SendBuffer), szCmd);
	
	memset(&m_RecvBuffer, 0, sizeof(m_RecvBuffer));
	ResetEvent(m_hEvtRecv);
	if(!(nRtn = send(m_sConnect, m_SendBuffer, strlen(m_SendBuffer), 0))) goto go_End;
	if(WaitForSingleObject(m_hEvtRecv, nTimeout*1000) == WAIT_TIMEOUT)
	{
		sprintf_s(szRespMsg, nLens, _T("Waitting for recv timeout, Recv: %s"), m_RecvBuffer);
		goto go_End;
	}

	if(strlen(m_RecvBuffer) <= nLens) strcpy_s(szRespMsg, nLens, m_RecvBuffer);
	else 
	{
		bRtn = false;
		sprintf_s(szRespMsg, nLens, _T("Buffer overflow over the length: %d"), strlen(m_RecvBuffer));
		OutputTraceLog(_T("Buffer overflow (over the length: %d), Recv: %s"), nLens, m_RecvBuffer);

		goto go_End;
	}

	bRtn = true;
go_End:
	if(m_pLogger) m_pLogger->OutputLog(TYPE_DETAILS, COLOR_WHITE, _T("> Send (%d): %s\r\n> %s"), nRtn, m_SendBuffer, m_RecvBuffer);

	ResetEvent(m_hEvtRecv);
	if(pCmdLen) delete []pCmdLen;
	return bRtn;
}

/*ONLY for GIT protocol*/
UINT mySocket::SocketRecvThread(LPVOID pParam)
{
	mySocket *pMain = (mySocket*)pParam;
	int nResult, nLen;
	int nCmdLens = 0;
	TCHAR szTmp[4096] = {'\0'}, szLength[4+1] = {'\0'};
	fd_set fdRead;
	struct timeval TimeVal;
	TimeVal.tv_sec = 1;
	TimeVal.tv_usec = 1000;	

	pMain->m_bExitThread = false;
	while(!pMain->m_bExitThread)
	{
		FD_ZERO(&fdRead);
		FD_SET(pMain->m_sConnect, &fdRead);

		nResult = select(pMain->m_sConnect+1, &fdRead, 0, 0, &TimeVal);
		if(nResult == -1) break;				//Error
		else if(nResult == 0) continue;			//Timeout
		else
		{
			if(FD_ISSET(pMain->m_sConnect, &fdRead))
			{
				memset(&szTmp, 0, 4096);

				if((nLen = recv(pMain->m_sConnect, szTmp, 4096, 0)) != 0)
				{
					/*Get Length*/
					if(!pMain->m_bLengthFlag)
					{
						strncpy_s(szLength, sizeof(szLength), szTmp, 4);
						nCmdLens = atol(szLength);

						pMain->m_bLengthFlag = true;
					}

					EnterCriticalSection(&pMain->m_cs);
					strcat_s(pMain->m_RecvBuffer, SOCKET_BUFFER_SIZE, szTmp);
					Sleep(1);
					LeaveCriticalSection(&pMain->m_cs);
				}	

				if((strlen(pMain->m_RecvBuffer)) == nCmdLens+4) SetEvent(pMain->m_hEvtRecv);
			}	
		}
	}

go_End:
	SetEvent(pMain->m_hEvtExitThread);
	return 0;
}