#include "Thread.h"
#include <handleapi.h>
#include <processthreadsapi.h>
#include <synchapi.h>
#include <winerror.h>

void Trace(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char buf[10240];
	vsprintf_s(buf, fmt, args);
	va_end(args);
#if 0
	va_list args;
	va_start(args, fmt);

	// Determine the required buffer size
	int size = vsnprintf(nullptr, 0, fmt, args) + 1; // +1 for null terminator
	va_end(args);

	if (size <= 0) {
		return; // Error determining buffer size
	}

	// Allocate buffer
	std::vector<char> buf(size);

	// Format the string
	va_start(args, fmt);
	vsnprintf(buf.data(), buf.size(), fmt, args);
	va_end(args);

	// Output the formatted string
	printf("%s", buf.data());
#endif
}

CThread::CThread()
{
	m_aRunning.store(false);
	m_hThread = INVALID_HANDLE_VALUE;
	m_pWorker.store(nullptr);
}

CThread::~CThread()
{
	Stop();
}

void CThread::ThreadEntry(void* arg)
{
	CThread* self = (CThread*)arg;
	if (self) self->ThreadWorker();
	_endthread();
}

bool CThread::IsValid()
{
	if (!m_hThread || m_hThread == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	// a thread is not in singled state means, it still running
	// otherwise, it has finished, been signaled notify others.
	return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
}

bool CThread::Start()
{
	m_hThread = (HANDLE)_beginthread(&CThread::ThreadEntry, 0, this);
	if (IsValid()) m_aRunning.store(true);
	return m_aRunning.load();
}

bool CThread::Stop()
{
	m_aRunning.store(false);
	TerminateThread(m_hThread, 0);
#if 0
	if (WaitForSingleObject(m_hThread, 500) != WAIT_OBJECT_0)
	{
		TerminateThread(m_hThread, 0);
	}
#endif
	m_hThread = INVALID_HANDLE_VALUE;
	return true;
}

bool CThread::IsIdle()
{
	auto pWorker = m_pWorker.load();
	bool ret = m_hThread == INVALID_HANDLE_VALUE || !pWorker || !pWorker->IsValid();
	return ret;
}

void CThread::UpdateWorker(const ::ThreadWorker& worker)
{
	if (!worker.IsValid()) return;
	m_pWorker.exchange(&worker);
}

void CThread::ThreadWorker()
{
	m_aRunning.store(true);
	while (m_aRunning.load())
	{
		auto pWorker = m_pWorker.load();
		if (!pWorker)
		{
			Sleep(1);
			continue;
		}
		if (pWorker->IsValid())
		{
			if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT)
			{
				// call worker method;
				int ret = (*pWorker)();
				if (ret != 0)
				{
					TRACE("ThreadWorker found warnning code %d\n", ret);
				}
				if (ret < 0)
				{
					m_pWorker.exchange(nullptr);
				}
			}
		}
		else
		{
			Sleep(1);
		}
	}
}