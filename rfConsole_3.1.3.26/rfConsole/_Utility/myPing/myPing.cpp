#include "StdAfx.h"
#include "myPing.h"


myPing::myPing(void)
{
}

myPing::~myPing(void)
{
}

void myPing::OutputTraceLog(TCHAR *format, ...)
{
#if (_DEBUG)
	TCHAR *pBuffer = new TCHAR[MAX_ARCBUFFER_SIZE];
	va_list marker;

	memset(pBuffer, 0, MAX_ARCBUFFER_SIZE);
	va_start(marker, format);
	vsprintf_s(pBuffer, MAX_ARCBUFFER_SIZE, format, marker);
	va_end(marker);

	OutputDebugString(pBuffer);	
	DEL_ARRAY(pBuffer)
#endif
}

bool myPing::PingUntilDead(LPSTR lpIP, int nPingTimeout, int nSucessCnt)
{
	bool bRtn = false, bPingSuccess = false;
	int nSuccessPing = 0, nCnt = 1;
	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	struct in_addr ReplyAddr;
	DWORD dwRetVal = 0;
	char SendData[32] = "Data Buffer";
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;

	if(lpIP)
	{
		if((ipaddr = inet_addr(lpIP)) == INADDR_NONE) 
		{
			OutputTraceLog(_T("Bad IP address: %s"), lpIP);
			goto go_End;
		}

		if((hIcmpFile = IcmpCreateFile()) == INVALID_HANDLE_VALUE)
		{
			OutputTraceLog(_T("call IcmpCreateFile fail: %ld"), GetLastError());
			goto go_End;
		}

		ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
		ReplyBuffer = (VOID*) malloc(ReplySize);

		printf(_T("    "));
		double dbStartTime = GetTickCount();
		do 
		{
			if((dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 1000)) != 0)
			{
				PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
				ReplyAddr.S_un.S_addr = pEchoReply->Address;

				if(pEchoReply->Status == IP_SUCCESS)
				{					
					Sleep(1000);
					nSuccessPing = 0;	/*Reset counter*/
					printf(".");					
				}
			}else
			{
				printf("*");
				if(++nSuccessPing >= nSucessCnt) 
				{
					bPingSuccess = true;
					break;
				}				
			}

			if((nCnt++%120) == 0) printf("\r\n    ");
			if(((GetTickCount() - dbStartTime)/1000) > nPingTimeout) break;
			
		} while (!bPingSuccess);

	}else OutputTraceLog(_T("Bad IP address: %s"), lpIP);

	bRtn = bPingSuccess;
go_End:
	if(ReplyBuffer) free(ReplyBuffer);
	return bRtn;
}

bool myPing::PingUntilAilve(LPSTR lpIP, int nPingTimeout, int nSucessCnt, int nPingInterval)
{
	bool bRtn = false, bPingSuccess = false;
	int nSuccessPing = 0, nCnt = 1;
	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	struct in_addr ReplyAddr;
	DWORD dwRetVal = 0;
	char SendData[32] = "Data Buffer";
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;

	if(lpIP)
	{
		if((ipaddr = inet_addr(lpIP)) == INADDR_NONE) 
		{
			OutputTraceLog(_T("Bad IP address: %s"), lpIP);
			goto go_End;
		}

		if((hIcmpFile = IcmpCreateFile()) == INVALID_HANDLE_VALUE)
		{
			OutputTraceLog(_T("call IcmpCreateFile fail: %ld"), GetLastError());
			goto go_End;
		}

		ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
		ReplyBuffer = (VOID*) malloc(ReplySize);

		printf(_T("    "));
		double dbStartTime = GetTickCount();
		do 
		{
			if((dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 1000)) != 0)
			{
				PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
				ReplyAddr.S_un.S_addr = pEchoReply->Address;

				if(pEchoReply->Status == IP_SUCCESS)
				{
					printf("*");

					if(++nSuccessPing >= nSucessCnt) 
					{
						bPingSuccess = true;
						break;
					}
					Sleep(nPingInterval);					
				}
			}else
			{
				/*if((nCnt%60) == 0) 
				{
					system(_T("arp -d"));
					printf(",");
				}else*/ printf(".");
				nSuccessPing = 0;	/*Reset counter*/				
			}
			
			if((nCnt++%120) == 0) printf("\r\n    ");
			if(((GetTickCount() - dbStartTime)/1000) > nPingTimeout) break;

		} while (!bPingSuccess);

	}else OutputTraceLog(_T("Bad IP address: %s"), lpIP);

	bRtn = bPingSuccess;
go_End:
	if(ReplyBuffer) free(ReplyBuffer);
	return bRtn;
}
