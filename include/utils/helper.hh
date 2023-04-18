#ifdef __GNUC__ // GCC, Clang, ICC
#define unreachable() __builtin_unreachable()
#elifdef _MSC_VER // MSVC
#define unreachable() __assume(false)
#endif
