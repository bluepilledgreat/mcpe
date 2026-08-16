#include "common/profiler/Profiler.hpp"
#include "common/threading/CThread.hpp"
#include "common/Utils.hpp"

const int MAX_DEPTH = 100;
const double MAX_TIME_BUCKET_OLDEST_TIME = 0.2;

ProfilerContextRegistry ProfilerContextRegistry::singleton;
ThreadLocal<ProfilerContextRoot> ProfilerThreadContext;

ProfilerContext::ProfilerContext(const std::string& name, ProfilerContextRoot* rootContext, ProfilerContext* parentContext, size_t childIndex)
	: m_name(name)
	, m_rootContext(rootContext)
#ifdef _PROFILER_PARENT_VALIDATION_
	, m_parentContext(parentContext)
#endif
	, m_childIndex(childIndex)
	, m_currentDepth(parentContext ? parentContext->m_currentDepth + 1 : 0)
	, m_currentMarker(nullptr)
{
}

ProfilerContext* ProfilerContext::getOrCreateContext(const std::string& name)
{
	{
		SharedLock<SharedMutex> lock(m_childrenMutex);

		for (std::vector<ProfilerContext*>::const_iterator it = m_children.begin(); it != m_children.end(); it++)
		{
			ProfilerContext* childContext = *it;
			if (childContext->getName() == name)
				return childContext;
		}
	}

	{
		UniqueLock<SharedMutex> lock(m_childrenMutex);

		size_t childIndex = m_children.size();
		ProfilerContext* newContext = new ProfilerContext(name, m_rootContext, this, childIndex);
		m_children.push_back(newContext);

		return newContext;
	}
}

void ProfilerContext::startProfiling()
{
	assert(m_rootContext != this); // Roots should not be calling this

	m_rootContext->pushToContextStack(this);
}

void ProfilerContext::stopProfiling(double endTime, double timeElapsed)
{
	assert(m_rootContext != this); // Roots should not be calling this
	assert(m_rootContext->peekFromContextStack() == this);

	m_rootContext->popFromContextStack();

	{
		LockGuard<Mutex> lock(m_timeBucketMutex);
		m_timeBucket.push_back(TimeBucketItem(endTime, timeElapsed));
		cleanupElapsedTimeBucket();
	}
}

void ProfilerContext::dumpInternal(ProfilerFunctionFrame& frame, ProfilerDepth_t& maxDepth)
{
	SharedLock<SharedMutex> lock(m_childrenMutex);

	if (m_currentDepth > maxDepth)
		maxDepth = m_currentDepth;

	bool isRoot = m_rootContext == this;

	frame.setName(m_name);
	frame.m_depth = m_currentDepth;
	frame.m_timeElapsed = computeElapsedTime();

	frame.m_childFrames.clear();
	frame.m_childFrames.reserve(m_children.size());

	for (std::vector<ProfilerContext*>::const_iterator it = m_children.begin(); it != m_children.end(); it++)
	{
		ProfilerContext* childContext = *it;
		if (isRoot || !childContext->m_timeBucket.empty())
		{
			frame.m_childFrames.push_back(ProfilerFunctionFrame());
			childContext->dumpInternal(frame.m_childFrames.back(), maxDepth);
		}
	}
}

double ProfilerContext::computeElapsedTime()
{
	double elapsedTime = 0.0;

	double currentTime = getTimeS();

	{
		LockGuard<Mutex> lock(m_timeBucketMutex);

		for (std::list<TimeBucketItem>::iterator it = m_timeBucket.begin(); it != m_timeBucket.end(); )
		{
			if (it->currentTime + MAX_TIME_BUCKET_OLDEST_TIME < currentTime)
				m_timeBucket.erase(it++);
			else
			{
				elapsedTime += it->elapsedTime;
				it++;
			}
		}
	}

	return elapsedTime;
}

void ProfilerContext::cleanupElapsedTimeBucket()
{
	double currentTime = getTimeS();

	for (std::list<TimeBucketItem>::iterator it = m_timeBucket.begin(); it != m_timeBucket.end(); )
	{
		if (it->currentTime + MAX_TIME_BUCKET_OLDEST_TIME < currentTime)
			m_timeBucket.erase(it++);
		else
			break;
	}
}

ProfilerContextRoot::ProfilerContextRoot()
	: ProfilerContext("<<<ROOT>>>", this, NULL, 0xFFFFFFFF)
	, m_threadId(CThread::GetCurrentThreadId())
{
	ProfilerContextRegistry::singleton.registerRoot(this);
}

ProfilerContextRoot::~ProfilerContextRoot()
{
	ProfilerContextRegistry::singleton.unregisterRoot(this);
}

ProfilerContext* ProfilerContextRoot::peekFromContextStack()
{
	return m_contextStack.size() != 0 ? m_contextStack.top() : this;
}

void ProfilerContextRoot::pushToContextStack(ProfilerContext* context)
{
	assert(context != this); // Roots should not be calling this

	m_contextStack.push(context);
}

void ProfilerContextRoot::popFromContextStack()
{
	if (m_contextStack.size() != 0)
		m_contextStack.pop();
}

void ProfilerContextRoot::dump(ProfilerFrame& frame)
{
	dumpInternal(frame, frame.m_maxDepth);

	// root contexts need their elapsed time calculated manually
	frame.m_timeElapsed = 0.0;
	for (std::vector<ProfilerFunctionFrame>::const_iterator it = frame.m_childFrames.begin(); it != frame.m_childFrames.end(); it++)
		frame.m_timeElapsed += it->m_timeElapsed;
}

void ProfilerContextRegistry::registerRoot(ProfilerContextRoot* contextRoot)
{
	UniqueLock<SharedMutex> lock(m_mutex);

	assert(std::find(m_contextRoots.begin(), m_contextRoots.end(), contextRoot) == m_contextRoots.end());
	m_contextRoots.push_back(contextRoot);
}

void ProfilerContextRegistry::unregisterRoot(ProfilerContextRoot* contextRoot)
{
	UniqueLock<SharedMutex> lock(m_mutex);

	std::vector<ProfilerContextRoot*>::iterator it = std::find(m_contextRoots.begin(), m_contextRoots.end(), contextRoot);
	assert(it != m_contextRoots.end());
	m_contextRoots.erase(it);
}

ProfilerMarker::ProfilerMarker(const char* markerName)
	: m_rootContext(ProfilerThreadContext.getLocal())
{
	ProfilerContext* parentContext = m_rootContext.peekFromContextStack();

	if (parentContext->getName() != markerName)
	{
		m_startTimeBeforeWork = getTimeS();
		m_context = parentContext->getOrCreateContext(markerName);

		assert(m_context->m_currentDepth <= MAX_DEPTH);

		m_rootContext.pushToContextStack(m_context);

		m_context->m_currentMarker = this;
		m_context->startProfiling();

		m_startTime = getTimeS();
	}
	else
	{
		// we are likely in a recursive function
		m_context = nullptr;
		// start time not set as it is not used
	}
}

ProfilerMarker::~ProfilerMarker()
{
	if (!m_context)
		return;

	double endTime = getTimeS();
	double timeElapsed = endTime - m_startTime;

	m_context->stopProfiling(endTime, timeElapsed);
	m_context->m_currentMarker = nullptr;

	m_rootContext.popFromContextStack();

	double endTimeAfterWork = getTimeS();

	double timeForStartWork = m_startTime - m_startTimeBeforeWork;
	double timeForEndWork = endTimeAfterWork - endTime;

	// negate from final result of the parent context
	ProfilerContext* parentContext = m_rootContext.peekFromContextStack();
	if (parentContext->m_currentMarker) // roots dont ever have markers
	{
		parentContext->m_currentMarker->m_startTime += timeForStartWork + timeForEndWork;
	}
}
