#pragma once
#include "compat/PlatformDefinitions.h"

#if MC_PLATFORM_UNIX

#include "common/threading/Semaphore_pthreads.hpp"

#elif defined(_WIN32)

#include "common/threading/Semaphore_win32.hpp"

#else

#error "Missing Semaphore implementation for platform"

#endif
