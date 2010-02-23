dnl $Id$
dnl config.m4 for extension barbershop

PHP_ARG_ENABLE(barbershop, whether to enable barbershop support,
dnl Make sure that the comment is aligned:
[  --enable-barbershop           Enable barbershop support])

if test "$PHP_BARBERSHOP" != "no"; then

  dnl # --with-barbershop -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/barbershop.h"  # you most likely want to change this
  dnl if test -r $PHP_BARBERSHOP/$SEARCH_FOR; then # path given as parameter
  dnl   BARBERSHOP_DIR=$PHP_BARBERSHOP
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for barbershop files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       BARBERSHOP_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$BARBERSHOP_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the barbershop distribution])
  dnl fi

  dnl # --with-barbershop -> add include path
  dnl PHP_ADD_INCLUDE($BARBERSHOP_DIR/include)

  dnl # --with-barbershop -> check for lib and symbol presence
  dnl LIBNAME=barbershop # you may want to change this
  dnl LIBSYMBOL=barbershop # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BARBERSHOP_DIR/lib, BARBERSHOP_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_BARBERSHOPLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong barbershop lib version or lib not found])
  dnl ],[
  dnl   -L$BARBERSHOP_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(BARBERSHOP_SHARED_LIBADD)

  PHP_NEW_EXTENSION(barbershop, barbershop.c, $ext_shared)
fi