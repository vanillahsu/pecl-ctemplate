dnl $Id: config.m4,v 1.7 2007/07/17 09:45:59 vanilla Exp $
dnl config.m4 for extension cTemplate

PHP_ARG_WITH(cTemplate, for cTemplate support,
Make sure that the comment is aligned:
[  --with-cTemplate             Include cTemplate support])

if test "$PHP_CTEMPLATE" != "no"; then
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/google/template.h"
  if test -r $PHP_CTEMPLATE/$SEARCH_FOR; then
    CTEMPLATE_DIR=$PHP_CTEMPLATE
  else
    AC_MSG_CHECKING([for cTemplate files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        CTEMPLATE_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  AC_MSG_CHECKING(for ZTS)
  zts=""
 
  AC_TRY_RUN([
#include "main/php_config.h"
int main (void)
{
  return (!ZTS);
}], no_zts=yes)

  if test "x$zts" = "x" ; then
      AC_MSG_RESULT(yes)
      PHP_ADD_LIBRARY_WITH_PATH(ctemplate, $CTEMPLATE_DIR/lib, CTEMPLATE_SHARED_LIBADD)
  else
      AC_MSG_RESULT(no)
      PHP_ADD_LIBRARY_WITH_PATH(ctemplate_nothreads, $CTEMPLATE_DIR/lib, CTEMPLATE_SHARED_LIBADD)
  fi
 
  if test -z "$CTEMPLATE_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the cTemplate distribution])
  fi

  PHP_ADD_INCLUDE($CTEMPLATE_DIR/include)

  PHP_REQUIRE_CXX
  PHP_SUBST(CTEMPLATE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(pthread)
  PHP_ADD_LIBRARY(stdc++)
  PHP_NEW_EXTENSION(cTemplate, cTemplate.cpp, $ext_shared)
  dnl CPPFLAGS="$CPPFLAGS -Werror -Wall -g"
fi
