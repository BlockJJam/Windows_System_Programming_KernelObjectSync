#include "stdafx.h"
#include "Windows.h"
#include "iostream"
using namespace std;

#define CMD_NONE	0
#define CMD_STR		1
#define CMD_POINT	2
#define CMD_TIME	3
#define CMD_EXIT	100

struct WAIT_ENV
{
	HANDLE	_hevSend;
	HANDLE	_hevResp;
	BYTE	_arBuff[256];
};

DWORD WINAPI WorkerProc(LPVOID pParam)
{
	WAIT_ENV* pwe = (WAIT_ENV*)pParam;
	DWORD dwThrId = GetCurrentThreadId();

	while (true)
	{
		DWORD dwWaitCode = WaitForSingleObject(pwe->_hevSend, INFINITE);
		if (dwWaitCode == WAIT_FAILED)
		{
			cout << " ~~~ WaitForSingleObject failed: " << GetLastError() << endl;
			break;
		}

		PBYTE pIter = pwe->_arBuff;
		LONG ICmd = *((PLONG)pIter); pIter += sizeof(LONG);
		if (ICmd == CMD_EXIT)
			break;

		LONG ISize = *((PLONG)pIter); pIter += sizeof(LONG);
		PBYTE pData = new BYTE[ISize + 1];
		memcpy(pData, pIter, ISize);
		SetEvent(pwe->_hevResp);

		switch (ICmd)
		{
		case CMD_STR:
		{
			pData[ISize] = 0;
			printf(" M== R-TH %d read STR : %s \n", dwThrId, pData);
		}
		break;

		case CMD_POINT:
		{
			PPOINT ppt = (PPOINT)pData;
			printf(" <== R-TH %d read POINT : (%d, %d)\n", dwThrId, ppt->x, ppt->y);
		}
		break;

		case CMD_TIME:
		{
			PSYSTEMTIME pst = (PSYSTEMTIME)pData;
			printf("<== R-TH %d readTime: %04d-%02d-%02d %02d:%02d:%02d+%03d \n", dwThrId, pst->wYear, pst->wMonth, pst->wDay, pst->wHour, pst->wMinute, pst->wSecond, pst->wMilliseconds);

		}
		break;
		}
		delete[] pData;
	}
	cout << "*** WorkerProc Thread Exits..." << endl;

	return 0;
}

void _tmain()
{
	cout << "========= Start EventNotify Test ========" << endl;

	WAIT_ENV we;
	we._hevSend = CreateEvent(NULL, FALSE, FALSE, NULL);
	we._hevResp = CreateEvent(NULL, FALSE, FALSE, NULL);

	DWORD dwThrId;
	HANDLE hThread = CreateThread(NULL, 0, WorkerProc, &we, 0, &dwThrId);

	char szIn[512];
	while (true)
	{
		cin >> szIn;
		if (_stricmp(szIn, "quit") == 0)
			break;

		LONG ICmd = CMD_NONE, ISize = 0;
		PBYTE pIter = we._arBuff + sizeof(LONG) * 2;

		if (_stricmp(szIn, "time") == 0)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			memcpy(pIter, &st, sizeof(st));
			ICmd = CMD_TIME, ISize = sizeof(st);
		}
		else if (_stricmp(szIn, "point") == 0)
		{
			POINT pt;
			pt.x = rand() % 1000; pt.y = rand() % 1000;
			*((PPOINT)pIter) = pt;
			ICmd = CMD_POINT, ISize = sizeof(pt);
		}
		else
		{
			ISize = strlen(szIn);
			memcpy(pIter, szIn, ISize);
			ICmd = CMD_STR;
		}
		((PLONG)we._arBuff)[0] = ICmd;
		((PLONG)we._arBuff)[1] = ISize;

		SignalObjectAndWait(we._hevSend, we._hevResp, INFINITE, FALSE);
	}

	*((PLONG)we._arBuff) = CMD_EXIT;
	SignalObjectAndWait(we._hevSend, hThread, INFINITE, FALSE);
	CloseHandle(hThread);
	CloseHandle(we._hevResp);
	CloseHandle(we._hevSend);
	
	cout << "========= End EventNotify Test ==========" << endl;
}