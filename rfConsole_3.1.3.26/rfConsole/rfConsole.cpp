// rfConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rfConsole.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

#define CONSOLE_HEIGHT	170
#define CONSOLE_WEIGHT	50
void setupConsole()
{
	BOOL ret;
	SMALL_RECT SR;

	CString strTmp;
	strTmp.Format(EDITOR_INFO, CCommFunc::m_ModuleVersion);
	SetConsoleTitle(strTmp.GetBuffer());

	SR.Top = 0;
	SR.Left = 0;
	SR.Bottom = CONSOLE_HEIGHT-1;
	SR.Right = CONSOLE_WEIGHT-1;
	if((ret=SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE),TRUE, &SR))) return;

	COORD Sz;
	Sz.X = CONSOLE_HEIGHT;
	Sz.Y = CONSOLE_WEIGHT;

	if((ret=SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),Sz))) return;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);
	HANDLE hMutex = CreateMutex(NULL, false, "Process");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		printf("The program is already running!!");
		CloseHandle(hMutex);
		hMutex = NULL;
	}else
	{
		if (hModule != NULL)
		{
			// initialize MFC and print and error on failure
			if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
			{
				// TODO: change error code to suit your needs
				_tprintf(_T("Fatal Error: MFC initialization failed\n"));
				nRetCode = 1;
			}
			else
			{
				TCHAR szTestFlow[MAX_PATH] = {'\0'};
				TCHAR szMAC[STRING_LENS_16] = {'\0'};
				TCHAR szSN[STRING_LENS_16] = {'\0'};
				bool bExit = false;
				HANDLE hCloseProgram = CreateEvent(NULL, TRUE, FALSE, NULL);

				//setupConsole();
				if(argc < 3) goto go_Usage;
				else
				{
					for(int n=0 ; n<argc ; n++)
					{
						if((strcmp(argv[n], "-RUN") == 0) || (strcmp(argv[n], "-run") == 0))
						{
							if(n+1 <= argc) strcpy_s(szTestFlow, MAX_PATH, argv[n+1]);
							else goto go_Usage;	
						}

						if(((strcmp(argv[n], "-MAC") == 0) || (strcmp(argv[n], "-mac") == 0)) && (n+1 <= argc)) strcpy_s(szMAC, STRING_LENS_16, argv[n+1]);
						if(((strcmp(argv[n], "-SN") == 0) || (strcmp(argv[n], "-sn") == 0)) && (n+1 <= argc)) strcpy_s(szSN, STRING_LENS_16, argv[n+1]);
						if((strcmp(argv[n], "-EXIT") == 0) || (strcmp(argv[n], "-exit") == 0)) bExit = true;
					}
					rfConsole(hCloseProgram, szTestFlow, szMAC, szSN, bExit);	
				}	
			}
		}
		else
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
			nRetCode = 1;
		}
	}	

	return nRetCode;
go_Usage:
	printf("Usage: rfConsole -RUN [file]\n\n[Options]\n-MAC\t\tSet MAC address\n-SN\t\tSet Serial Number\n");
	return nRetCode;
}

void rfConsole(HANDLE hCloseProgram, LPSTR lpTestFlow, LPSTR lpMAC, LPSTR lpSN, bool bExit)
{
	CTestManager Tester;
	Tester.Initialize(hCloseProgram, lpTestFlow, lpMAC, lpSN, bExit);

	if(Tester.StartRun())
	{
		while(WaitForSingleObject(hCloseProgram, 1000) == WAIT_TIMEOUT) Sleep(100);
	}
	Tester.Terminate();

	Sleep(10);
}