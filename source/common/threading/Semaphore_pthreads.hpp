#pragma once

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <cassert>
#include "Semaphore_base.hpp"
#include "Mutex.hpp"

class Semaphore
{
private:
	// disable copy constructors
	Semaphore(const Semaphore&);
	Semaphore& operator=(const Semaphore&);

public:
	Semaphore(uint32_t initialCount, uint32_t maximumCount)
		: m_count(initialCount), m_maxCount(maximumCount)
	{
		assert(maximumCount > 0);
		assert(initialCount <= maximumCount);

		int result = pthread_cond_init(&m_cond, nullptr);
		assert(result == 0);
	}

	Semaphore(uint32_t initialCount)
		: m_count(initialCount), m_maxCount(INT32_MAX)
	{
		int result = pthread_cond_init(&m_cond, nullptr);
		assert(result == 0);
	}

	~Semaphore()
	{
		int result = pthread_cond_destroy(&m_cond);
		assert(result == 0);
	}

public:
	void wait()
	{
		LockGuard<Mutex> lock(m_mutex);

		int result;
		// while loop catches POSIX spurious wakeups
		while (m_count == 0)
		{
			// Thanks to 'friend', we can access the inner m_mutex
			result = pthread_cond_wait(&m_cond, &m_mutex.m_mutex);
			assert(result == 0);
		}
		--m_count;
	}

	bool wait(uint32_t millisecondsTimeout)
	{
		// Convert relative ms to absolute timespec for condition variable
		struct timeval tv;
		gettimeofday(&tv, nullptr);

		struct timespec ts;
		ts.tv_sec = tv.tv_sec + (millisecondsTimeout / 1000);
		ts.tv_nsec = (tv.tv_usec * 1000) + ((millisecondsTimeout % 1000) * 1000000);

		// Handle nanosecond carry-over logic cleanly
		if (ts.tv_nsec >= 1000000000)
		{
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000;
		}

		LockGuard<Mutex> lock(m_mutex);
		bool acquired = false;

		int waitResult;
		while (m_count == 0)
		{
			waitResult = pthread_cond_timedwait(&m_cond, &m_mutex.m_mutex, &ts);
			if (waitResult == ETIMEDOUT)
				break;
		}

		if (m_count > 0)
		{
			--m_count;
			acquired = true;
		}

		return acquired;
	}

	bool release()
	{
		return release(1);
	}

	bool release(uint32_t count)
	{
		if (count == 0) return false;

		LockGuard<Mutex> lock(m_mutex);

		if (m_maxCount - m_count < count)
			return false;

		m_count += count;

		int result;
		if (count == 1)
		{
			result = pthread_cond_signal(&m_cond);
		}
		else
		{
			// Broadcast wakes up all waiting threads if we release multiple units
			result = pthread_cond_broadcast(&m_cond);
		}
		assert(result == 0);

		return true;
	}

private:
	Mutex m_mutex;
	pthread_cond_t m_cond;
	uint32_t m_count;
	uint32_t m_maxCount;
};