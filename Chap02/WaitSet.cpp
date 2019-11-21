#include "stdafx.h"
#include "Windows.h"
#include "WaitPool.h"

#include "WaitItem.h"
#include "WaitSet.h"

DWORD WINAPI WaitSet::WaitSetProc(PVOID pParam)
{
	PWAIT_SET pws = (PWAIT_SET)pParam;
	pws->InnerWaitSetProc();
	return 0;
}

void WaitSet::Init()
{
	m_noti = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_resp = CreateEvent(NULL, FALSE, FALSE, NULL);

	DWORD dwThrId;
	m_thread = CreateThread(NULL, 0, WaitSetProc, this, 0, &dwThrId);
}

void WaitSet::Release()
{
	Command(CMDT::EXIT, NULL);

	CloseHandle(m_thread);
	CloseHandle(m_noti);
	CloseHandle(m_resp);
}

void WaitSet::Command(CMDT cmd, WAIT_ITEM* arg)
{
	m_cmd = cmd, m_arg = arg;

	SignalObjectAndWait
	(
		m_noti, (cmd == CMDT::EXIT) ? m_thread : m_resp, INFINITE, FALSE
	);
}

void WaitSet::InnerWaitSetProc()
{
	HANDLE		arWaits[MAXIMUM_WAIT_OBJECTS] = { NULL, };
	PWAIT_ITEM	arIetms[MAXIMUM_WAIT_OBJECTS] = { NULL, };

	arWaits[0] = m_noti;
	arIetms[0] = NULL;
	while (true)
	{
		DWORD dwWaitCode = WaitForMultipleObjects(m_count + 1, arWaits, FALSE, INFINITE);
		if (dwWaitCode == WAIT_FAILED)
			break;


		if (dwWaitCode == WAIT_OBJECT_0)
		{
			if (m_cmd == CMDT::EXIT)
				break;

			if (m_cmd == CMDT::NEW)
			{
				arWaits[m_count + 1] = m_arg->_handle;
				arIetms[m_count + 1] = m_arg;
				m_count++;
			}
			else
			{
				int nldx = 1;
				for (; nldx < m_count + 1; nldx++)
				{
					if (m_arg == arIetms[nldx])
						break;
				}
				if (nldx < m_count + 1)
				{
					for (int i = nldx; i < m_count; i++)
					{
						arWaits[i] = arWaits[i + 1];
						arIetms[i] = arIetms[i + 1];

					}
				}
			}
			SetEvent(m_resp);
		}
		else
		{
			int nldx = (int)dwWaitCode - WAIT_OBJECT_0;
			PWAIT_ITEM pwi = arIetms[nldx];
			pwi->_pfnCB(pwi->_pPrm);
		}
	}
}