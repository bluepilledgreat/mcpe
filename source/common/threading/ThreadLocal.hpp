#pragma once
#include "compat/PlatformDefinitions.h"

#if MC_PLATFORM_UNIX

// pthread approach (Unix)
#include "common/threading/ThreadLocal_pthread.hpp"

#else

// FLS approach (Windows)
#include "common/threading/ThreadLocal_fls.hpp"

#endif
