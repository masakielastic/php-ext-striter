PHP_ARG_ENABLE(striter, whether to enable striter support,
[  --enable-striter        Enable striter support])

if test "$PHP_STRITER" != "no"; then
  PHP_NEW_EXTENSION(striter, striter.c string_iterator.c, $ext_shared)
fi