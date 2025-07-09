PHP_ARG_ENABLE(striter, whether to enable striter support,
[  --enable-striter        Enable striter support])

if test "$PHP_STRITER" != "no"; then
  PHP_CHECK_LIBRARY(pcre2-8, pcre2_compile_8, [
    PHP_ADD_LIBRARY(pcre2-8, 1, STRITER_SHARED_LIBADD)
    AC_DEFINE(HAVE_PCRE2, 1, [Have PCRE2 library])
    
    # Check for JIT support
    PHP_CHECK_LIBRARY(pcre2-8, pcre2_jit_compile, [
      AC_DEFINE(HAVE_PCRE2_JIT, 1, [Have PCRE2 JIT support])
    ], [], [-lpcre2-8])
  ], [
    AC_MSG_ERROR([PCRE2 library not found. Please install libpcre2-dev])
  ])
  
  PHP_NEW_EXTENSION(striter, striter.c string_iterator.c, $ext_shared)
  PHP_SUBST(STRITER_SHARED_LIBADD)
fi