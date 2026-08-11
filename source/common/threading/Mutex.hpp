#pragma once
#include "compat/PlatformDefinitions.h"

#if MC_PLATFORM_UNIX

#include "common/threading/Mutex_pthread.hpp"

#elif defined(_WIN32)

#include "common/threading/Mutex_win32.hpp"

#else

#error "Missing Mutex implementation for platform"

#endif

template<typename T>
class LockGuard
{
private:
	T& m_mutex;

private:
	// disable copy constructors
	LockGuard(const LockGuard&);
	LockGuard& operator=(const LockGuard&);

public:
	LockGuard(T& mutex)
		: m_mutex(mutex)
	{
		m_mutex.lock();
	}

	~LockGuard()
	{
		m_mutex.unlock();
	}
};
