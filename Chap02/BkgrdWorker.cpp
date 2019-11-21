#include "stdafx.h"
#include "BkgrdWorker.h"
#include "Windows.h"
#include "iostream"
using namespace std;

WNDPROC BkgrdWorker::sm_pfnOldWP;
LONG	BkgrdWorker::sm_spRefCnt;
UINT	BkgrdWorker::WM_BKW_REPORT;
UINT	BKgrdWorker::WM_BKW_COMPLETED;

BkgrdWorker::BkgrdWorker()
{
	CancellationPending = false;
	IsBusy = false;

	m_hThread = NULL;
	m_hWnd = NULL;

	m_pfnDoWork = NULL;
	m_pfnReport = NULL;
	m_pfnCompleted = NULL;
}

BkgrdWorker::~BkgrdWorker()
{
}

DWORD WINAPI BkgrdWorker::BkgrdWorkerProc(PVOID pParam)
{
	BkgrdWorker* pThis = (BkgrdWorker*)pParam;
	pThis->InnerBkgrdWorkerProc();
	return 0;
}

void BkgrdWorker::InnerBkgrdWorkerProc()
{
	HRESULT hr = S_OK;
	try
	{
		m_pfnDoWork(this, &m_argDoWork);
	}
	catch (HRESULT e)
	{
		hr = e;
	}

	if (m_pfnCompleted)
	{
		RunWorkerCompletedArgs* pArg = new RunWorkerCompletedArgs(m_argDoWork.Result, hr, m_argDoWork.Cancel);
		if (m_hWnd != NULL)
			SendMessage(m_hWnd, WM_BKW_COMPLETED, (WPARAM)this, (LPARAM)pArg);
		else
			m_pfnCompleted(this, pArg);
	}
	memset(&m_argDoWork, 0, sizeof(m:_argDoWork));
	CancellationPending = false;
	IsBusy = false;
}

LRESULT CALLBACK BkgrdWorker::BkgrdWndProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_BKW_REPORT || uMsg == WM_BKW_COMPLETED)
	{
		BkgrdWorker* pThis = (BkgrdWorker*)wParam;
		if (uMsg == WM_BKW_REPORT)
		{
			ProgressChangedArgs* pArg = (ProgressChangedArgs*)lParam;
			pThis->m_pfnReport(pThis, pArg);
			delete pArg;
			pThis->IsBusy = false;
		}
		return 0;
	}

	return CallWindowProc(sm_pfnOldWP, hWnd, uMsg, wParam, lParam);
}


void BkgrdWorker::Initiailize(HWND hWnd, DoWorkHandler pfnDoWork)
{
	if (hWnd != NULL && sm_spRefCnt == 0)
	{
		sm_pfnOldWP = (WNNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)BkgrdWndProckHook);
		sm_spRefCnt++;

		WM_BKW_REPORT = RegisterWindowMessage(_T("BkgrdWorker::WM_BKW_REPORT"));
		WM_BKW_COMPLETED = RegisterWindowMessage(_T("BkgrdWorker::WM_BKW_COMPLETED"));
	}
	m_hWnd = hWnd;
	m_pfnDoWork = pfnDoWork;
}

void BkgrdWorker::Uninitialize()
{
	if (m_hThread != NULL)
	{
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	if (sm_spRefCnt > 0)
	{
		sm_spRefCnt--;
		if (sm_spRefCnt == 0)
		{
			SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)sm_pfnOldWP);
			sm_pfnOldWp = NULL;
		}
	}

	m_hWnd = NULL;
	m_pfnDoWork = NULL;
	m_pfnReport = NULL;
	m_pfnCompleted = NULL;
}

void BkgrdWorker::RunWorkerAsync(PVOID pParam)
{
	if (m_hThread != NULL)
		CloseHandle(m_hThread);

	m_argDoWork.Argument = pParam;
	DWORD dwThreadId = 0;
	m_hThread = CreateThread(NULL, 0, BkgrdWorkerProc, this, 0, &dwThreadId);
	IsBusy = true;
}

void BkgrdWorker::ReportProgress(int nPercent, PVOID pParam)
{
	if (m_pfnReport)
	{
		ProgressChangedArgs* pArg = new ProgressChangedArgs(nPercent, pParam);
		if (m_hWnd != NULL)
			SendMessage(m_hWnd, WM_BKW_REPORT, (WPARAM)this, (LPARAM)pArg);
		else
			m_pfnReport(this, pArg);
	}
}

void BkgWorker::CancelAsync()
{
	CancellationPending = true;
}
