//
//  ScopeContext.hpp
//  NBCraft
//
//  Created by Brent on 8/22/26.
//
//

#pragma once

#include "CompilerDefinitions.h"

#define _MC_SCOPE_STR_LITERAL( x ) # x
#define _MC_SCOPE_STR_EXPANDED( x ) _MC_SCOPE_STR_LITERAL( x )
#define _MC_SCOPE_STR_WITH_QUOTE( x ) _MC_SCOPE_STR_EXPANDED( x )


#if MC_COMPILER_MSVC
// MSVC includes the class name here
#define MC_SCOPE_FUNC __FUNCTION__
#else // !MC_COMPILER_MSVC
#define MC_SCOPE_FUNC _MC_SCOPE_STR_WITH_QUOTE(MC_SCOPE_CLASS) "::" __FUNCTION__
#endif