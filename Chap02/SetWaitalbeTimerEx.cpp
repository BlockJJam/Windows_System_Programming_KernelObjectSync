#include "stdafx.h"
#include "Windows.h"
#include "iostream"
using namespace std;

#include "PowrProf.h"
#pragma comment(lib, "PowrProf.lib")
#pragma warning( disable :4996)

void _tmain()
{
	SYSTEMTIME st;
	HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

	LARGE_INTEGER lUTC;
	lUTC.QuadPart = -300000000LL;

	char text[] = "Continuous measurement";
	wchar_t wtext[30];
	mbstowcs(wtext, text, strlen(text) + 1);//Plus null
	LPWSTR ptr = wtext;
	REASON_CONTEXT rc;
	rc.Version = POWER_REQUEST_CONTEXT_VERSION;
	rc.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
	rc.Reason.SimpleReasonString = ptr;

	SetWaitableTimerEx(hTimer, &lUTC, 30000, NULL, NULL, &rc, 1000);
	GetLocalTime(&st);
	printf("Timer Set Call : %04d/%02d/%02d-%02d:%02d:%02d+%03d\n",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	SetSuspendState(TRUE, FALSE, FALSE);

	int nSnoozeCnt = 0;
	while (nSnoozeCnt < 5)
	{
		WaitForSingleObject(hTimer, INFINITE);

		GetLocalTime(&st);
		printf("Timer Signaled : %04d/%02d/%02d-%02d:%02d:%02d+%03d\n",
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		if (nSnoozeCnt == 0)
			MessageBeep(MB_ICONWARNING);
		nSnoozeCnt++;
	}

	getchar();
	CloseHandle(hTimer);
}

