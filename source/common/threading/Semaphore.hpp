#pragma once

#ifdef _XBOX
#include <xtl.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <cstdint>
#include <cassert>

class Semaphore
{
private:
	HANDLE m_handle;

public:
	Semaphore(int32_t initialCount, int32_t maximumCount)
		: m_handle(CreateSemaphoreA(NULL, initialCount, maximumCount, NULL))
	{
		assert(m_handle);
	}

	Semaphore(int32_t initialCount)
		: m_handle(CreateSemaphoreA(NULL, initialCount, INT32_MAX, NULL))
	{
		assert(m_handle);
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

	bool release(int32_t count)
	{
		return ReleaseSemaphore(m_handle, count, NULL) == TRUE;
	}
};
