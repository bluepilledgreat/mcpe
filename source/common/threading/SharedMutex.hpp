#pragma once
#include "Mutex.hpp"
#include "Semaphore.hpp"
#include <cassert>

class SharedMutex
{
private:
	uint32_t m_readersCount;
	Mutex m_readersCountMutex;
	Semaphore m_writeSemaphore;
	Semaphore m_entryTurnstile;

private:
	// disable copy constructors
	SharedMutex(const SharedMutex&);
	SharedMutex& operator=(const SharedMutex&);

public:
	SharedMutex()
		: m_writeSemaphore(1, 1)
		, m_entryTurnstile(1, 1)
		, m_readersCount(0)
	{
	}

	~SharedMutex()
	{
		assert(m_readersCount == 0);
	}

public:
	void readLock()
	{
		m_entryTurnstile.wait();
		m_entryTurnstile.release();

		LockGuard<Mutex> lock(m_readersCountMutex);

		if (++m_readersCount == 1)
			m_writeSemaphore.wait();
	}

	void readUnlock()
	{
		LockGuard<Mutex> lock(m_readersCountMutex);

		if (--m_readersCount == 0)
			m_writeSemaphore.release();
	}

	void writeLock()
	{
		// block new read locks
		m_entryTurnstile.wait();

		m_writeSemaphore.wait();
	}

	void writeUnlock()
	{
		m_writeSemaphore.release();

		m_entryTurnstile.release();
	}
};

template<typename T>
class SharedLock
{
private:
	T& m_mutex;

private:
	// disable copy constructors
	SharedLock(const SharedLock&);
	SharedLock& operator=(const SharedLock&);

public:
	SharedLock(T& mutex)
		: m_mutex(mutex)
	{
		m_mutex.readLock();
	}

	~SharedLock()
	{
		m_mutex.readUnlock();
	}
};

template<typename T>
class UniqueLock
{
private:
	T& m_mutex;

private:
	// disable copy constructors
	UniqueLock(const UniqueLock&);
	UniqueLock& operator=(const UniqueLock&);

public:
	UniqueLock(T& mutex)
		: m_mutex(mutex)
	{
		m_mutex.writeLock();
	}

	~UniqueLock()
	{
		m_mutex.writeUnlock();
	}
};
