#pragma once
#include <pthread.h>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>
#include "common/threading/Mutex.hpp"

template<typename T>
class ThreadLocal
{
public:
	typedef T* (*CreatorFunction_t)();

private:
	static std::map<T*, ThreadLocal<T>*> Owners;
	static Mutex OwnersMutex;

private:
	pthread_key_t m_key;
	CreatorFunction_t m_creatorFunction;
	std::vector<T*> m_pool;
	Mutex m_poolMutex;

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
		T* obj = reinterpret_cast<T*>(ptr);

		{
			ThreadLocal<T>* owningLocal;

			{
				LockGuard<Mutex> lock(OwnersMutex);

				typename std::map<T*, ThreadLocal<T>*>::iterator it = Owners.find(obj);
				assert(it != Owners.end());
				owningLocal = it->second;
				Owners.erase(it);
			}

			{
				LockGuard<Mutex> lock(owningLocal->m_poolMutex);

				typename std::vector<T*>::iterator it = std::find(owningLocal->m_pool.begin(), owningLocal->m_pool.end(), obj);
				assert(it != m_pool.end());
				owningLocal->m_pool.erase(it);
			}
		}

		delete obj;
	}

public:
	ThreadLocal()
		: m_creatorFunction(_Create)
	{
		if (pthread_key_create(&m_key, _Destroy) != 0)
			throw std::runtime_error("pthread_key_create failed");
	}

	ThreadLocal(CreatorFunction_t creatorFunction)
		: m_creatorFunction(creatorFunction)
	{
		if (pthread_key_create(&m_key, _Destroy) != 0)
			throw std::runtime_error("pthread_key_create failed");
	}

	~ThreadLocal()
	{
		int result = pthread_key_delete(m_key);
		assert(result == 0);

		{
			LockGuard<Mutex> lock(OwnersMutex);

			for (typename std::vector<T*>::iterator it = m_pool.begin(); it != m_pool.end(); it++)
			{
				typename std::map<T*, ThreadLocal<T>*>::iterator mapIt = Owners.find(*it);
				assert(mapIt != Owners.end());
				Owners.erase(mapIt);
			}
		}

		{
			for (typename std::vector<T*>::iterator it = m_pool.begin(); it != m_pool.end(); it++)
				delete (*it);
		}
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
			LockGuard<Mutex> lock(OwnersMutex);

			Owners[ptr] = this;
		}

		{
			LockGuard<Mutex> lock(m_poolMutex);

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

		pthread_setspecific(m_key, nullptr);
	}
};
