//
//  CompilerDefinitions.h
//  NBCraft
//
//  Created by Brent on 8/22/26.
//
//

#pragma once

/* Microsoft - Microsoft Visual C++ (MSVC) */
#if (defined(_MSC_VER))
#define MC_COMPILER_MSVC 1
#else
#define MC_COMPILER_MSVC 0
#endif
