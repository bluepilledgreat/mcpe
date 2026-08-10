#pragma once
#include <pthread.h>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <algorithm>

template<typename T>
class ThreadLocal
{
private:
	pthread_key_t m_key;
	T* (*m_creatorFunction)();
	std::vector<T*> m_pool;
	std::mutex m_poolMutex;

private:
	// disable copy constructors
	ThreadLocal(const ThreadLocal&);
	ThreadLocal& operator=(const ThreadLocal&);

private:
	static T* _Create()
	{
		return new T();
	}

	static void _Destroy(void* ptr)
	{
		delete reinterpret_cast<T*>(ptr);
	}

public:
	ThreadLocal()
		: m_creatorFunction(_Create)
	{
		if (pthread_key_create(&m_key, _Destroy) != 0)
			throw std::runtime_error("pthread_key_create failed");
	}

	ThreadLocal(T* (*creatorFunction)())
		: m_creatorFunction(creatorFunction)
	{
		if (pthread_key_create(&m_key, _Destroy) != 0)
			throw std::runtime_error("pthread_key_create failed");
	}

	~ThreadLocal()
	{
		int result = pthread_key_delete(m_key);
		assert(result == 0);

		for (typename std::vector<T*>::iterator it = m_pool.begin(); it != m_pool.end(); it++)
			delete (*it);
	}

private:
	T* _get() const
	{
		return reinterpret_cast<T*>(pthread_getspecific(m_key));
	}

public:
	T& getLocal()
	{
		T* storedPtr = _get();
		if (storedPtr)
			return *storedPtr;

		T* ptr = m_creatorFunction();
		int result = pthread_setspecific(m_key, ptr);
		if (result != 0)
		{
			delete ptr;
			throw std::runtime_error("pthread_setspecific failed");
		}

		{
			std::lock_guard<std::mutex> lock(m_poolMutex);

			assert(std::find(m_pool.begin(), m_pool.end(), ptr) == m_pool.end());
			m_pool.push_back(ptr);
		}

		return *ptr;
	}

	T* getLocalPtr()
	{
		return &getLocal();
	}

	void resetLocal()
	{
		T* storedPtr = _get();
		if (!storedPtr)
			return;

		{
			std::lock_guard<std::mutex> lock(m_poolMutex);

			typename std::vector<T*>::iterator it = std::find(m_pool.begin(), m_pool.end(), storedPtr);
			assert(it != m_pool.end());
			m_pool.erase(it);
		}

		pthread_setspecific(m_key, nullptr);
	}
};
