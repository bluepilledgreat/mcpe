#pragma once
#include "compat/PlatformDefinitions.h"

#if MC_PLATFORM_UNIX

// pthread approach (Unix)
#include "common/threading/ThreadLocal_pthread.hpp"

#else

// TLS approach (Windows XP and newer)
#include "common/threading/ThreadLocal_tls.hpp"

// FLS approach (Windows Vista and newer)
//#include "common/threading/ThreadLocal_fls.hpp"

#endif
