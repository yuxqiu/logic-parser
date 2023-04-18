#pragma once

#ifdef __GNUC__ // GCC, Clang, ICC
#define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER) // MSVC
#define unreachable() __assume(false)
#endif
