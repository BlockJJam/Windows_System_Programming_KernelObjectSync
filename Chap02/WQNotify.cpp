#include "stdafx.h"
#include "Windows.h"
#include "list"
#include "iostream"
using namespace std;

#define CMD_NONE	0
#define CMD_STR		1
#define CMD_POINT	2
#define CMD_TIME	3
#define CMD_EXIT	100

#pragma warning( disable :4996)
class WAIT_QUE
{
	struct NOTI_ITEM
	{
		LONG	_cmd, _size;
		PBYTE	_data;

		NOTI_ITEM()
		{
			_cmd = _size = 0, _data = NULL;
		}
		NOTI_ITEM(LONG cmd, LONG size, PBYTE data)
		{
			_cmd = cmd, _size = size, _data = data;
		}

	};
	typedef std::list<NOTI_ITEM> ITEM_QUE;

	HANDLE m_hMutx;
	HANDLE m_hSema;
	ITEM_QUE m_queue;

public:
	WAIT_QUE()
	{
		m_hMutx = m_hSema = NULL;
	}
	~WAIT_QUE()
	{
		if (m_hMutx != NULL) CloseHandle(m_hMutx);
		if (m_hSema != NULL) CloseHandle(m_hSema);
	}

public:
	void Init()
	{
		m_hSema = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
		m_hMutx = CreateMutex(NULL, FALSE, NULL);
	}

	void Enqueue(LONG cmd, LONG size = 0, PBYTE data = NULL)
	{
		NOTI_ITEM ni(cmd, size, data);

		WaitForSingleObject(m_hMutx, INFINITE);
		m_queue.push_back(ni);
		ReleaseMutex(m_hMutx);

		ReleaseSemaphore(m_hSema, 1, NULL);
	}

	PBYTE Dequeue(LONG& cmd, LONG& size)
	{
		DWORD dwWaitCode = WaitForSingleObject(m_hSema, INFINITE);
		if (dwWaitCode == WAIT_FAILED)
		{
			cmd = CMD_NONE, size = HRESULT_FROM_WIN32(GetLastError());
			return NULL;
		}

		NOTI_ITEM ni;
		WaitForSingleObject(m_hMutx, INFINITE);
		ITEM_QUE::iterator it = m_queue.begin();
		ni = *it;
		m_queue.pop_front();
		ReleaseMutex(m_hMutx);

		cmd = ni._cmd, size = ni._size;
		return ni._data;
	}
};

DWORD WINAPI WorkerProc(LPVOID pParam)
{
	WAIT_QUE* pwq = (WAIT_QUE*)pParam;
	DWORD dwThrId = GetCurrentThreadId();

	while (true)
	{
		LONG ICmd, ISize;
		PBYTE pData = pwq->Dequeue(ICmd, ISize);
		if (ISize < 0)
		{
			cout << " ~~~ WaitForSingleObject failed : " << GetLastError() << endl;
			break;
		}
		if (ICmd == CMD_EXIT)
			break;
		
		switch (ICmd)
		{
		case CMD_STR:
		{
			pData[ISize] = 0;
			printf(" <== R-Th %d read STR : %s\n", dwThrId, pData);
		}
		break;
		case CMD_POINT:
		{
			PPOINT ppt = (PPOINT)pData;
			printf("  <== R-TH %d read POINT : (%d, %d)\n", dwThrId, ppt->x, ppt->y);
		}
		break;

		case CMD_TIME:
		{
			PSYSTEMTIME pst = (PSYSTEMTIME)pData;
			printf(" <== R-TH %d read TIME : %04d-%02d-%02d : %02d:%02d:%02d+%03d\n", dwThrId, pst->wYear, pst->wMonth, pst->wDay, pst->wHour, pst->wMinute, pst->wSecond, pst->wMilliseconds);
		}
		break;

		}
		delete[] pData;
	}
	cout << " *** WorkerProc Thread exits .. " << endl;
	return 0;
}

void _tmain()
{
	cout << "======== start WQueNotify test ========" << endl;
	
	WAIT_QUE wq;
	wq.Init();

	DWORD dwThrID;
	HANDLE hThread = CreateThread(NULL, 0, WorkerProc, &wq, 0, &dwThrID);

	char szIn[512];
	while (true)
	{
		cin >> szIn;
		if (_stricmp(szIn, "quit") == 0)
			break;

		LONG ICmd = CMD_NONE, ISize = 0;
		PBYTE pData = NULL;
		
		if (_stricmp(szIn, "time") == 0)
		{
			ISize = sizeof(SYSTEMTIME), ICmd = CMD_TIME;
			pData = new BYTE[ISize];

			SYSTEMTIME st;
			GetLocalTime(&st);
			memcpy(pData, &st, ISize);
		}
		else if (_stricmp(szIn, "point") == 0)
		{
			ISize = sizeof(POINT), ICmd = CMD_POINT;
			pData = new BYTE[ISize];

			POINT pt;
			pt.x = rand() % 1000; pt.y = rand() % 1000;
			*((PPOINT)pData) = pt;
		}
		else
		{
			ISize = strlen(szIn), ICmd = CMD_STR;
			pData = new BYTE[ISize + 1];
			strcpy((char*)pData, szIn);
		}

		wq.Enqueue(ICmd, ISize, pData);
	}

	wq.Enqueue(CMD_EXIT);
	WaitForSingleObject(hThread, INFINITE);

	cout << "======== end WQueNotify Test =======" << endl;
}