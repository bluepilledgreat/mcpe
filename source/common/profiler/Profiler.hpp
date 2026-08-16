#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include "common/threading/CThread.hpp"
#include "common/threading/ThreadLocal.hpp"
#include "common/threading/Mutex.hpp"
#include "common/threading/SharedMutex.hpp"

#ifdef _DEBUG
#define _PROFILER_PARENT_VALIDATION_
#endif

struct TimeBucketItem
{
	double currentTime;
	double elapsedTime;

	TimeBucketItem(double currentTime, double elapsedTime)
		: currentTime(currentTime)
		, elapsedTime(elapsedTime)
	{
	}
};

typedef uint16_t ProfilerDepth_t;

class ProfilerContext;
class ProfilerContextRoot;
class ProfilerMarker;

class ProfilerFunctionFrame
{
	friend class ProfilerContext;
	friend class ProfilerContextRoot;

private:
	std::string m_name;
	double m_timeElapsed;
	ProfilerDepth_t m_depth;
	std::vector<ProfilerFunctionFrame> m_childFrames;

public:
	ProfilerFunctionFrame()
		: m_timeElapsed(0.0)
		, m_depth(0)
	{
	}

	~ProfilerFunctionFrame()
	{
	}

private:
	void setName(const std::string& name)
	{
		if (m_name != name)
			m_name = name;
	}

public:
	const std::string& getName() const
	{
		return m_name;
	}

	double getElapsedTime() const
	{
		return m_timeElapsed;
	}

	ProfilerDepth_t getDepth() const
	{
		return m_depth;
	}

	const std::vector<ProfilerFunctionFrame>& getChildFrames() const
	{
		return m_childFrames;
	}
};

class ProfilerFrame : public ProfilerFunctionFrame
{
	friend class ProfilerContextRoot;

private:
	ProfilerDepth_t m_maxDepth;

public:
	ProfilerFrame()
		: m_maxDepth(0)
	{
	}

	~ProfilerFrame()
	{
	}

public:
	ProfilerDepth_t getMaxDepth() const
	{
		return m_maxDepth;
	}
};

class ProfilerContext
{
	friend class ProfilerMarker;

private:
	std::string m_name;
	size_t m_childIndex;
	ProfilerContextRoot* m_rootContext;
#ifdef _PROFILER_PARENT_VALIDATION_
	ProfilerContext* m_parentContext;
#endif
	ProfilerDepth_t m_currentDepth;
	std::vector<ProfilerContext*> m_children;
	std::list<TimeBucketItem> m_timeBucket;
	SharedMutex m_childrenMutex;
	Mutex m_timeBucketMutex;
	ProfilerMarker* m_currentMarker;

public:
	ProfilerContext(const std::string& name, ProfilerContextRoot* rootContext, ProfilerContext* parentContext, size_t childIndex);

private:
	ProfilerContext* getOrCreateContext(const std::string& name);

	void startProfiling();
	void stopProfiling(double endTime, double timeElapsed);

private:
	double computeElapsedTime();
	void cleanupElapsedTimeBucket();

protected:
	void dumpInternal(ProfilerFunctionFrame& frame, ProfilerDepth_t& maxDepth);

public:
	const std::string& getName() const
	{
		return m_name;
	}
};

class ProfilerContextRoot : public ProfilerContext
{
	friend class ProfilerContext;
	friend class ProfilerMarker;

private:
	CThread::ID m_threadId;
	std::stack<ProfilerContext*> m_contextStack;

public:
	ProfilerContextRoot();
	~ProfilerContextRoot();

private:
	ProfilerContext* peekFromContextStack();
	void pushToContextStack(ProfilerContext* context);
	void popFromContextStack();

public:
	CThread::ID getThreadId() const
	{
		return m_threadId;
	}

	void dump(ProfilerFrame& frame);
};

class ProfilerContextRegistry
{
	friend class ProfilerContextRoot;

private:
	std::vector<ProfilerContextRoot*> m_contextRoots;

public:
	SharedMutex m_mutex;

public:
	ProfilerContextRegistry() {}

public:
	// Requires read lock
	const std::vector<ProfilerContextRoot*>& getContextRoots() const
	{
		return m_contextRoots;
	}

private:
	void registerRoot(ProfilerContextRoot* contextRoot);
	void unregisterRoot(ProfilerContextRoot* contextRoot);

public:
	static ProfilerContextRegistry singleton;
};

class ProfilerMarker
{
private:
	double m_startTimeBeforeWork;
	double m_startTime;
	ProfilerContextRoot& m_rootContext;
	ProfilerContext* m_context;

public:
	ProfilerMarker(const char* markerName);
	~ProfilerMarker();
};

extern ThreadLocal<ProfilerContextRoot> ProfilerThreadContext;

#define PROFILE_FUNCTION() ProfilerMarker _profiler_function_marker_(__FUNCTION__)
//#define PROFILE_FUNCTION() 
