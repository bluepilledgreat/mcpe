#pragma once

#ifdef _XBOX
#include <xtl.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <cassert>

#include "Semaphore_base.hpp"

class Semaphore : public Semaphore_base
{
private:
	void _init(uint32_t initialCount, uint32_t maximumCount = UINT32_MAX)
	{
		m_handle = CreateSemaphore(NULL, initialCount, maximumCount, NULL);
		assert(m_handle);
	}

public:
	Semaphore(uint32_t initialCount, uint32_t maximumCount)
		: Semaphore_base(initialCount, maximumCount)
	{
		_init(initialCount, maximumCount);
	}

	Semaphore(uint32_t initialCount)
		: Semaphore_base(initialCount)
	{
		_init(initialCount);
	}

public:
	void wait()
	{
		DWORD result = WaitForSingleObject(m_handle, INFINITE);
		assert(result == WAIT_OBJECT_0);
	}

	bool wait(int32_t millisecondsTimeout)
	{
		DWORD result = WaitForSingleObject(m_handle, millisecondsTimeout);
		assert(result != WAIT_FAILED);
		return result == WAIT_OBJECT_0;
	}

	bool release()
	{
		return ReleaseSemaphore(m_handle, 1, NULL) == TRUE;
	}

	bool release(uint32_t count)
	{
		return ReleaseSemaphore(m_handle, count, NULL) == TRUE;
	}

private:
	HANDLE m_handle;
};
