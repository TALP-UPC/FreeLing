#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([FreeLing],[4.0])
AC_CONFIG_SRCDIR([src/libfreeling/version.cc])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign])

AC_PREREQ([2.68])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

## All macosx systems will put in host_os something like:
## darwin15.4.0 (this for El Capitan)

case "$host_os" in
 *darwin*)
   AC_DEFINE(MACOSX)
   system="macosx"
   ;;
 *)
   system="linux"
   ;;
esac

using_xpressive="no"
using_kaldi="no"
### using_boost_locale="no"

######## Begin checkings ##########

AC_ARG_ENABLE([xpressive_regex],
              [AS_HELP_STRING([--enable-xpressive-regex],[Use boost::xpressive instead of default boost::regex. Default is: Not enabled. Not compatible with --enable-boost-locale. If you are in MacOSX, do not enable it.])],
              [if test "$enableval" = yes; then
                 # define symbol for CPP
                 AC_DEFINE(USE_XPRESSIVE_REGEX)
		 # inform other checks below
                 using_xpressive="yes"
               fi
               # pass info to automake
               AM_CONDITIONAL([USE_XPRESSIVE_REGEX],[test "$enableval" = yes])
              ],
              [AM_CONDITIONAL([USE_XPRESSIVE_REGEX],[test 0 = 1])])

AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug],[Compile with debug information. Default is: Not enabled.])],
              [if test "$enableval" = yes; then
                 # define symbol for CPP
                 AC_DEFINE(DEBUG)
               fi
               # pass info to automake
               AM_CONDITIONAL([DEBUG],[test "$enableval" = yes])
              ],
              [AM_CONDITIONAL([DEBUG],[test 0 = 1])])

AC_ARG_ENABLE([traces],
              [AS_HELP_STRING([--enable-traces],[Compile with tracing information. Slows down the system, but you can know what's going on. Default is: Not enabled.])],
              [if test "$enableval" = yes; then
                 # define symbol for CPP
                 AC_DEFINE(VERBOSE)
               fi
               # pass info to automake
               AM_CONDITIONAL([VERBOSE],[test "$enableval" = yes])
              ],
              [AM_CONDITIONAL([VERBOSE],[test 0 = 1])])

AC_ARG_ENABLE([warnings],
              [AS_HELP_STRING([--disable-warnings],[Compile without warnings. Useful for large explotation systems with no logging desired. Default is: Enabled.])],
              [if test "$enableval" = no; then
                 # define symbol for CPP
                 AC_DEFINE(NO_WARNINGS)
               fi
               # pass info to automake
               AM_CONDITIONAL([NO_WARNINGS],[test "$enableval" = no])
              ],
              [AM_CONDITIONAL([NO_WARNINGS],[test 1 = 0])])

AC_ARG_ENABLE([build-tests],
              [AS_HELP_STRING([--enable-build-tests],[Compile unittest programs. Default: Not compiled.])],
              [if test "$enableval" = yes; then
                 # define symbol for CPP
                 AC_DEFINE(BUILD_TESTS)
               fi
               # pass info to automake
               AM_CONDITIONAL([BUILD_TESTS],[test "$enableval" = yes])
              ],
              [AM_CONDITIONAL([BUILD_TESTS],[test 0 = 1])])

AC_ARG_ENABLE([download-embeddings],
              [AS_HELP_STRING([--enable-download-embeddings],[Download and install word embedding data files. Default: Not installed.])],
              [AM_CONDITIONAL([GET_EMBEDDINGS],[test "$enableval" = yes])],
              [AM_CONDITIONAL([GET_EMBEDDINGS],[test 0 = 1])])

AC_ARG_WITH([kaldi-asr],
              [AS_HELP_STRING([--with-kaldi-asr],[Compile with kaldi, specifying kaldi's installation root folder as an argument. Default: Not compiled.])],
              [
                 # define symbol for CPP
                 AC_DEFINE(KALDI)
                 # with-kaldi-asr = /path/to/kaldi
                 AC_SUBST([KALDI_ROOT],["$withval"])
                 # pass info to automake
                 AM_CONDITIONAL([KALDI],[test 1 = 1])
                 using_kaldi="yes"
              ],
              [
                AM_CONDITIONAL([KALDI],[test 0 = 1])
              ])


# preprocessor and compiler flags
CXXFLAGS="-std=gnu++0x -Wall -Wno-sign-compare -fPIC"
CFLAGS="-Wall -Wno-sign-compare"
# Checks for programs.
AC_PROG_CXX

LT_INIT
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LANG([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <list>
                                     using namespace std;]],
                                   [[list<int> lst; int s;
                                     for (auto &i : lst) s+=i;]]
                                   )],
                  [echo "checking for required compiler C++11 support... yes"],
                  [echo "checking for required compiler C++11 support... no" &&
                   echo ""
                   echo "  Your compiler does not suppport some C++11 features required to compile FreeLing." &&
                   echo "  Version g++ >= 4.6 is needed." &&
                   echo "" && exit 1])

# init variables
AM_CONDITIONAL([BOOST_MT],[test 1 = 0])
AM_CONDITIONAL([BOOST_GCC],[test 1 = 0])

# check headers

if test $using_xpressive = "yes"
then
AC_CHECK_HEADERS([boost/xpressive/xpressive.hpp],[],
                      [echo "" && echo "   boost/xpressive/xpressive.hpp not found." &&
                      echo "   Make sure libboost headers (package libboost-dev or the like)" &&
                      echo "   are installed and can be found in a standard path." &&
                      echo "   You may need to set CPPFLAGS to specify search path." &&
                      echo "" && exit 1])
else
AC_CHECK_HEADERS([boost/regex.hpp],[],[echo "" && echo "   boost/regex.hpp not found." &&
                      echo "   Make sure libboost headers (package libboost-dev or the like)" &&
                      echo "   are installed and can be found in a standard path." &&
                      echo "   You may need to set CPPFLAGS to specify search path." &&
                      echo "" && exit 1])
AC_CHECK_HEADERS([boost/regex/icu.hpp],[],[echo "" && echo "   boost/regex/icu.hpp not found." &&
                      echo "   Make sure libicu headers (package libicu-dev or the like)" &&
                      echo "   are installed and can be found in a standard path." &&
                      echo "   You may need to set CPPFLAGS to specify search path." &&
                      echo "" && exit 1])
fi

AC_CHECK_HEADERS([boost/program_options.hpp],[],[echo "" && echo "   boost/program_options not found." &&
                      echo "   Make sure libboost headers (package libboost-dev or the like)" &&
                      echo "   are installed and can be found in a standard path." &&
                      echo "   You may need to set CPPFLAGS to specify search path." &&
                      echo "" && exit 1])
AC_CHECK_HEADERS([boost/thread.hpp],[],[echo "" && echo "   boost/thread.hpp not found." &&
                      echo "   Make sure libboost headers (package libboost-dev or the like)" &&
                      echo "   are installed and can be found in a standard path." &&
                      echo "   You may need to set CPPFLAGS to specify search path." &&
                      echo "" && exit 1])
AC_CHECK_HEADERS([boost/thread/mutex.hpp],[],[echo "" && echo "   boost/thread/mutex.hpp not found." &&
                      echo "   Make sure libboost headers (package libboost-dev or the like)" &&
                      echo "   are installed and can be found in a standard path." &&
                      echo "   You may need to set CPPFLAGS to specify search path." &&
                      echo "" && exit 1])
AC_CHECK_HEADERS([zlib.h],[],[echo "" && echo "   zlib.h not found." &&
                      echo "   Make sure zlib headers (package zlib-dev or the like)" &&
                      echo "   are installed and can be found in a standard path." &&
                      echo "   You may need to set CPPFLAGS to specify search path." &&
                      echo "" && exit 1])

# Checks for libraries.

## find out if boost version is older than 1.42
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/version.hpp>
                                  #if BOOST_VERSION < 104200
                                  #error Version older than 1.42
                                  #endif]],
                                [[]])],
                [echo "checking for boost version... >= 1.42";AM_CONDITIONAL([BOOST_OLD],[test 0 = 1])],
                [echo "checking for boost version... < 1.42";AM_CONDITIONAL([BOOST_OLD],[test 1 = 1])])

if test $using_xpressive != "yes"
then
AC_CHECK_LIB([icuuc],[main],
                     [echo "OK">/dev/null;],
                     [echo "" &&
                      echo "   Unicode ICU library not found." &&
                      echo "   Make sure libicu is" &&
                      echo "   installed and can be found in a standard path." &&
                      echo "   You may need to set LDFLAGS to specify search path." &&
                      echo "" && exit 1])

AC_CHECK_LIB([boost_regex],[main],
                      [MTLIB=""; echo "OK">/dev/null;],
                      AC_CHECK_LIB([boost_regex-mt],[main],
                                   [MTLIB="-mt"; echo "OK">/dev/null;AM_CONDITIONAL([BOOST_MT],[test 1 = 1])],
                                   AC_CHECK_LIB([boost_regex-gcc-mt],[main],
                                                [MTLIB="-gcc-mt"; echo "OK">/dev/null;AM_CONDITIONAL([BOOST_GCC],[test 1 = 1])],
                                                [echo "" &&
                                                 echo "   boost_regex library not found." &&
                                                 echo "   Make sure libboost_regex is" &&
                                                 echo "   installed and can be found in a standard path." &&
                                                 echo "   You may need to set LDFLAGS to specify search path." &&
                                                 echo "" && exit 1])))

LDF_BK=$LIBS
LIBS=" $LDF_BK -lboost_regex$MTLIB"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/regex/icu.hpp>]],
                               [[boost::u32regex re=boost::make_u32regex(L"");]])],
                [echo "OK">/dev/null;],
                [LIBS=" $LDF_BK -lboost_regex-mt" &&
                 AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/regex/icu.hpp>]],
                                               [[boost::u32regex re=boost::make_u32regex(L"");]])],
                                [echo "OK">/dev/null;],
                                [LIBS=" $LDF_BK -lboost_regex-gcc-mt" &&
                                 AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/regex/icu.hpp>]],
                                                            [[boost::u32regex re=boost::make_u32regex(L"");]])],
                                               [echo "OK">/dev/null;],
                                               [echo "" &&
                                                echo "   libboost_regex does not include Unicode ICU support." &&
                                                echo "   Make sure that the installed libboost_regex was built" &&
                                                echo "   with libicu support." &&
                                                echo "" && exit 1])])])
LIBS=$LDF_BK
fi

AC_CHECK_LIB([boost_program_options],[main],
                      [echo "OK">/dev/null;],
                       AC_CHECK_LIB([boost_program_options-mt],[main],
                                    [echo "OK">/dev/null;AM_CONDITIONAL([BOOST_MT],[test 1 = 1])],
                                     AC_CHECK_LIB([boost_program_options-gcc-mt],[main],
                                                  [echo "OK">/dev/null;AM_CONDITIONAL([BOOST_GCC],[test 1 = 1])],
                                                  [echo "" &&
                                                   echo "   boost_program_options library not found." &&
                                                   echo "   Make sure libboost_program_options is" &&
                                                   echo "   installed and can be found in a standard path." &&
                                                   echo "   You may need to set LDFLAGS to specify search path." &&
                                                   echo "" && exit 1])))

AC_CHECK_LIB([boost_system],[main],
                      [MTLIB=""; echo "OK">/dev/null;],
                      AC_CHECK_LIB([boost_system-mt],[main],
                                   [MTLIB="-mt"; echo "OK">/dev/null;AM_CONDITIONAL([BOOST_MT],[test 1 = 1])],
                                    AC_CHECK_LIB([boost_system-gcc-mt],[main],
                                                 [MTLIB="-gcc-mt"; echo "OK">/dev/null;AM_CONDITIONAL([BOOST_GCC],[test 1 = 1])],
                                                 [echo "" &&
                                                  echo "   boost_system library not found." &&
                                                  echo "   Make sure libboost_system is" &&
                                                  echo "   installed and can be found in a standard path." &&
                                                  echo "   You may need to set LDFLAGS to specify search path." &&
                                                  echo "" && exit 1])))

LIBS=" $LDF_BK -lboost_system$MTLIB"
AC_CHECK_LIB([boost_thread],[main],
                      [MTLIB=""; echo "OK">/dev/null;],
                       AC_CHECK_LIB([boost_thread-mt],[main],
                                    [MTLIB="-mt"; echo "OK">/dev/null;AM_CONDITIONAL([BOOST_MT],[test 1 = 1])],
                                     AC_CHECK_LIB([boost_thread-gcc-mt],[main],
                                                  [MTLIB="-gcc-mt"; echo "OK">/dev/null;AM_CONDITIONAL([BOOST_GCC],[test 1 = 1])],
                                                  [echo "" &&
                                                   echo "   boost_thread library not found." &&
                                                   echo "   Make sure libboost_thread is" &&
                                                   echo "   installed and can be found in a standard path." &&
                                                   echo "   You may need to set LDFLAGS to specify search path." &&
                                                   echo "" && exit 1])))
LIBS=$LDF_BK

AC_CHECK_LIB([z],[main],[echo "OK">/dev/null;], [echo "" &&
                                                 echo "   zlib library not found." &&
                                                 echo "   Make sure libz is installed and can" &&
                                                 echo "   be found in a standard path." &&
                                                 echo "   You may need to set LDFLAGS to specify search path." &&
                                                 echo "" && exit 1])

if test $using_kaldi = "yes"
then
LDF=$LDFLAGS
CPF=$CPPFLAGS
CXF=$CXXFLAGS
LDFLAGS="$LDFLAGS -L$KALDI_ROOT/tools/openfst/lib -L$KALDI_ROOT/src/lib"
CPPFLAGS="$CPPFLAGS -I$KALDI_ROOT/tools/ATLAS/include -I$KALDI_ROOT/src/ -I$KALDI_ROOT/tools/openfst/include"
CXXFLAGS="$CXXFLAGS -Wno-unused-local-typedefs"
AC_CHECK_HEADERS([$KALDI_ROOT/src/base/kaldi-common.h],[
  AC_CHECK_LIB([kaldi-base],[main],[
    AC_CHECK_LIB([fst],[main],[echo "OK">/dev/null;], 
                              [echo "" &&
                               echo "   OpenFst not found in the specified directory." &&
                               echo "   Make sure that you executed the install instruction" &&
                               echo "   in the tools directory of your kaldi installation. " &&
                               echo "" && exit 1]) 
                 ],
                 [echo "" &&
                  echo "   Kaldi shared library not found in the specified directory." &&
                  echo "   Make sure the directory specified with option '--with-kaldi-asr'" &&
                  echo "   contains a compiled kaldi installation, with the option " &&
                  echo "   --shared used in the configure step." &&
                  echo "" && exit 1])
        ],
        [echo "" &&
         echo "   Kaldi installation not found in the specified directory." &&
         echo "   Make sure the directory specified with option '--with-kaldi-asr'" &&
         echo "   contains a correct kaldi installation." &&
         echo "" && exit 1])
LDFLAGS=$LDF
CPPFLAGS=$CPF
CXXFLAGS=$CXF
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_STDC


AC_SUBST(CPPFLAGS)

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 src/Makefile
                 src/include/Makefile
                 src/libfreeling/Makefile
                 src/libasr/Makefile
                 src/libfoma/Makefile
                 src/libtreeler/Makefile
                 src/libtreeler/main/Makefile
                 src/main/Makefile
                 src/utilities/Makefile
                 src/tests/Makefile
                 freeling.pc])

AC_OUTPUT
