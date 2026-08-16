#pragma once

#include <stdint.h>

class Semaphore_base
{
public:
	Semaphore_base(uint32_t initialCount, uint32_t maximumCount) {}
	Semaphore_base(uint32_t initialCount) {}

public:
	void wait() {}
	bool wait(uint32_t millisecondsTimeout) {}
	bool release() {}
	bool release(uint32_t count) {}
};
