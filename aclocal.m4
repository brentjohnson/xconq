AC_DEFUN(CY_AC_PATH_TCLCONFIG, [
#
# Ok, lets find the tcl configuration
# First, look for one uninstalled.  
# the alternative search directory is invoked by --with-tclconfig
#

if test x"${no_tcl}" = x ; then
  # we reset no_tcl in case something fails here
  no_tcl=true
  AC_ARG_WITH(tclconfig, [  --with-tclconfig=DIR    Directory containing tcl configuration (tclConfig.sh)],
         with_tclconfig=${withval})
  AC_MSG_CHECKING([for Tcl configuration])
  AC_CACHE_VAL(ac_cv_c_tclconfig,[

  # First check to see if --with-tclconfig was specified.
  if test x"${with_tclconfig}" != x ; then
    if test -f "${with_tclconfig}/tclConfig.sh" ; then
      ac_cv_c_tclconfig=`(cd ${with_tclconfig}; pwd)`
    else
      AC_MSG_ERROR([${with_tclconfig} directory doesn't contain tclConfig.sh])
    fi
  fi

  # then check for a private Tcl installation
  if test x"${ac_cv_c_tclconfig}" = x ; then
    for i in \
		./tcl \
		../tcl \
		`ls -dr ../tcl[[7-9]]* 2>/dev/null` \
		../../tcl \
		`ls -dr ../../tcl[[7-9]]* 2>/dev/null` \
		../../../tcl \
		`ls -dr ../../../tcl[[7-9]]* 2>/dev/null` ; do
      if test -f "$i/${configdir}/tclConfig.sh" ; then
        ac_cv_c_tclconfig=`(cd $i/${configdir}; pwd)`
	break
      fi
    done
  fi
  # check in a few common install locations
  if test x"${ac_cv_c_tclconfig}" = x ; then
    tcl_probe_dirs="${prefix}/lib /usr/lib /usr/local/lib /mingw/lib"
    for i in `ls -d ${tcl_probe_dirs} 2>/dev/null` ; do
      if test -f "$i/tclConfig.sh" ; then
        ac_cv_c_tclconfig=`(cd $i; pwd)`
	break
      fi
    done
  fi
  # check in some other known public install locations
  # NOTE: This could possibly pick up wrong version if more than one 
  #  Tcl/Tk version is installed in this form.
  if test x"${ac_cv_c_tclconfig}" = x ; then
    for i in \
        `ls -d ${prefix}/lib/tcl[[7-9]]* /usr/lib/tcl[[7-9]]* 2>/dev/null` ; do
      if test -f "$i/tclConfig.sh" ; then
        ac_cv_c_tclconfig=`(cd $i; pwd)`
        break
      fi
    done
  fi
  # check in a few other private locations
  if test x"${ac_cv_c_tclconfig}" = x ; then
    for i in \
        ${srcdir}/../tcl \
	`ls -dr ${srcdir}/../tcl[[7-9]]* 2>/dev/null` ; do
      if test -f "$i/${configdir}/tclConfig.sh" ; then
        ac_cv_c_tclconfig=`(cd $i/${configdir}; pwd)`
	break
      fi
    done
  fi
  ])
  if test x"${ac_cv_c_tclconfig}" = x ; then
    TCLCONFIG="# no Tcl configs found"
    AC_MSG_WARN(Can't find Tcl configuration definitions)
  else
    no_tcl=
    TCLCONFIG=${ac_cv_c_tclconfig}/tclConfig.sh
    AC_MSG_RESULT(found $TCLCONFIG)
  fi
fi
])

AC_DEFUN(CY_AC_LOAD_TCLCONFIG, [
    . $TCLCONFIG

    AC_SUBST(TCL_VERSION)
    AC_SUBST(TCL_MAJOR_VERSION)
    AC_SUBST(TCL_MINOR_VERSION)
    AC_SUBST(TCL_CC)
    AC_SUBST(TCL_DEFS)
    AC_SUBST(TCL_UNSHARED_LIB_SUFFIX)
    AC_SUBST(TCL_EXTRA_CFLAGS)


dnl ## On some versions of Cygwin, the 'TCL_INCLUDE_SPEC' is incorrectly 
dnl ##  indicated to be '/nonexistent/include'. We must be on the look out 
dnl ##  for this, and doubt it if we see it.
    if test "x${TCL_INCLUDE_SPEC}" = "x-I/nonexistent/include" \
	    -a ! -d /nonexistent/include ;
    then
	TCL_INCLUDE_SPEC=""
    fi
dnl ## If TCL_INCLUDE_SPEC is not defined, then we might be on a Debian 
dnl ##  system, and should try to synthesize it.
    if test "x${TCL_INCLUDE_SPEC}" = "x" ; 
    then
      if test "x${TCL_INC_DIR}" != "x" ;
      then
        if test -d ${TCL_INC_DIR} ;
        then
          TCL_INCLUDE_SPEC="-I${TCL_INC_DIR}"
	fi
      fi
    fi
dnl ## If neither TCL_INCLUDE_SPEC nor TCL_INC_DIR are defined, we still 
dnl ##  might be on a Debian system. Try another synthesis.
    if test "x${TCL_INCLUDE_SPEC}" = "x" ; 
    then
      if test "x${TCL_VERSION}" != "x" -a "x${TCL_PREFIX}" != "x" ;
      then
	if test -d ${TCL_PREFIX}/include/tcl${TCL_VERSION} ; 
	then
          TCL_INCLUDE_SPEC="-I${TCL_PREFIX}/include/tcl${TCL_VERSION}"
	fi
      fi
    fi
dnl ## Filter out common include paths to prevent <> vs. "" conflicts.
    TCL_INCLUDE_SPEC=`echo ${TCL_INCLUDE_SPEC} | grep -v '/usr/include$'`
    AC_SUBST(TCL_INCLUDE_SPEC)

    if test "x${TCL_LDFLAGS}" = "x" ; then
      TCL_LDFLAGS=${TCL_LIB_SPEC}
    fi
dnl ## Fairly crude hack to find the lib under certain conditions.
    if test "x${TCL_LDFLAGS}" = "x" ; then
      tcl_libs=`ls ./tcl[[7-9]]*.lib ./libtcl[[7-9]]*.a 2>/dev/null`
      if test "x${tcl_libs}" != "x" ; then
        TCL_LDFLAGS="../${tcl_libs}"
      fi 
    fi
    if test "x${TCL_LDFLAGS}" = "x" ; then
      tcl_libs=`ls tcltk/tcl[[7-9]]*.lib tcltk/libtcl[[7-9]]*.a 2>/dev/null`
      if test "x${tcl_libs}" != "x" ; then
	TCL_LDFLAGS="../${tcl_libs}"
      fi 
    fi
    if test "x${TCL_LDFLAGS}" = "x" ; then
      tcl_libs=`ls tcl/tcl[[7-9]]*.lib tcl/libtcl[[7-9]]*.a 2>/dev/null`
      if test "x${tcl_libs}" != "x" ; then
	TCL_LDFLAGS="../${tcl_libs}"
      fi 
    fi
dnl ## Another attempt to salvage a usable lib.
    if test "x${TCL_LDFLAGS}" = "x" ; then
      if test "x${TCL_LIB_FILE}" != "x" ; then
        TCL_LDFLAGS="`dirname ${TCLCONFIG}`/${TCL_LIB_FILE}"
      fi
    fi
    dnl Tack on extra goodies that may be needed.
dnl Let the TK_LIBS do the talking. At worst, TCL_LIBS is a redundant 
dnl  subset of TK_LIBS; at best, it is nothing.
dnl    if test "x${TCL_LIBS}" != "x" ; then
dnl      TCL_LDFLAGS="${TCL_LDFLAGS} ${TCL_LIBS}"
dnl    fi

dnl not used, don't export to save symbols
dnl    AC_SUBST(TCL_LIB_FILE)

dnl don't export, not used outside of configure
dnl     AC_SUBST(TCL_LIBS)
dnl not used, don't export to save symbols
dnl    AC_SUBST(TCL_PREFIX)

dnl not used, don't export to save symbols
dnl    AC_SUBST(TCL_EXEC_PREFIX)

    AC_SUBST(TCL_SHLIB_CFLAGS)
    AC_SUBST(TCL_SHLIB_LD)
dnl don't export, not used outside of configure
    AC_SUBST(TCL_SHLIB_LD_LIBS)
    AC_SUBST(TCL_SHLIB_SUFFIX)
dnl not used, don't export to save symbols
    AC_SUBST(TCL_DL_LIBS)
    AC_SUBST(TCL_LD_FLAGS)
dnl don't export, not used outside of configure
    AC_SUBST(TCL_LD_SEARCH_FLAGS)
    AC_SUBST(TCL_COMPAT_OBJS)
    AC_SUBST(TCL_RANLIB)
    AC_SUBST(TCL_BUILD_LIB_SPEC)
    AC_SUBST(TCL_LIB_SPEC)
    AC_SUBST(TCL_LIB_VERSIONS_OK)

dnl not used, don't export to save symbols
dnl    AC_SUBST(TCL_SHARED_LIB_SUFFIX)

dnl not used, don't export to save symbols
dnl    AC_SUBST(TCL_UNSHARED_LIB_SUFFIX)

	AC_SUBST(TCL_LDFLAGS)
])

AC_DEFUN(CY_AC_PATH_TKCONFIG, [
#
# Ok, lets find the tk configuration
# First, look for one uninstalled.  
# the alternative search directory is invoked by --with-tkconfig
#

if test x"${no_tk}" = x ; then
  # we reset no_tk in case something fails here
  no_tk=true
  AC_ARG_WITH(tkconfig, [  --with-tkconfig=DIR     Directory containing tk configuration (tkConfig.sh)],
         with_tkconfig=${withval})
  AC_MSG_CHECKING([for Tk configuration])
  AC_CACHE_VAL(ac_cv_c_tkconfig,[

  # First check to see if --with-tkconfig was specified.
  if test x"${with_tkconfig}" != x ; then
    if test -f "${with_tkconfig}/tkConfig.sh" ; then
      ac_cv_c_tkconfig=`(cd ${with_tkconfig}; pwd)`
    else
      AC_MSG_ERROR([${with_tkconfig} directory doesn't contain tkConfig.sh])
    fi
  fi

  # then check for a private Tk library
  if test x"${ac_cv_c_tkconfig}" = x ; then
    for i in \
		./tk \
		../tk \
		`ls -dr ../tk[[4-9]]* 2>/dev/null` \
		../../tk \
		`ls -dr ../../tk[[4-9]]* 2>/dev/null` \
		../../../tk \
		`ls -dr ../../../tk[[4-9]]* 2>/dev/null` ; do
      if test -f "$i/${configdir}/tkConfig.sh" ; then
        ac_cv_c_tkconfig=`(cd $i/${configdir}; pwd)`
	break
      fi
    done
  fi
  # check in a few common install locations
  if test x"${ac_cv_c_tkconfig}" = x ; then
    tk_probe_dirs="${prefix}/lib /usr/lib /usr/local/lib /mingw/lib"
    for i in `ls -d ${tk_probe_dirs} 2>/dev/null` ; do
      if test -f "$i/tkConfig.sh" ; then
        ac_cv_c_tkconfig=`(cd $i; pwd)`
	break
      fi
    done
  fi
  # check in some other known public install locations
  # NOTE: This could possibly pick up wrong version if more than one 
  #  Tcl/Tk version is installed in this form.
  if test x"${ac_cv_c_tkconfig}" = x ; then
    for i in \
        `ls -d ${prefix}/lib/tk[[7-9]]* /usr/lib/tk[[7-9]]* 2>/dev/null` ; do
      if test -f "$i/tkConfig.sh" ; then
        ac_cv_c_tkconfig=`(cd $i; pwd)`
        break
      fi
    done
  fi
  # check in a few other private locations
  if test x"${ac_cv_c_tkconfig}" = x ; then
    for i in \
		${srcdir}/../tk \
		`ls -dr ${srcdir}/../tk[[4-9]]* 2>/dev/null` ; do
      if test -f "$i/${configdir}/tkConfig.sh" ; then
        ac_cv_c_tkconfig=`(cd $i/${configdir}; pwd)`
	break
      fi
    done
  fi
  ])
  if test x"${ac_cv_c_tkconfig}" = x ; then
    TKCONFIG="# no Tk configs found"
    AC_MSG_WARN(Can't find Tk configuration definitions)
  else
    no_tk=
    TKCONFIG=${ac_cv_c_tkconfig}/tkConfig.sh
    AC_MSG_RESULT(found $TKCONFIG)
  fi
fi

])

AC_DEFUN(CY_AC_LOAD_TKCONFIG, [
    if test -f "$TKCONFIG" ; then
      . $TKCONFIG
    fi

dnl ## If TK_INCLUDE_SPEC is not defined, then we might be on a Debian 
dnl ##  system and have need of it, and should try to synthesize it.
dnl ## On most other systems the TCL_INCLUDE_SPEC is enough to pick up 
dnl ##  Tk headers.
    if test "x${TK_INCLUDE_SPEC}" = "x" ; 
    then
      if test "x${TK_INC_DIR}" != "x" ; 
      then
	if test -d ${TK_INC_DIR} ; 
	then
          TK_INCLUDE_SPEC="-I${TK_INC_DIR}"
	fi
      fi
    fi
dnl ## If neither TK_INCLUDE_SPEC nor TK_INC_DIR are defined, we still 
dnl ##  might be on a Debian system. Try another synthesis.
    if test "x${TK_INCLUDE_SPEC}" = "x" ; 
    then
      if test "x${TK_VERSION}" != "x" -a "x${TK_PREFIX}" != "x" ;
      then
	if test -d ${TK_PREFIX}/include/tk${TK_VERSION} ; 
	then
          TK_INCLUDE_SPEC="-I${TK_PREFIX}/include/tk${TK_VERSION}"
	fi
      fi
    fi
dnl ## Filter out common include paths to prevent <> vs. "" conflicts.
    TK_INCLUDE_SPEC=`echo ${TK_INCLUDE_SPEC} | grep -v '/usr/include$'`
    AC_SUBST(TK_INCLUDE_SPEC)

    if test "x${TK_LDFLAGS}" = "x" ; then
      TK_LDFLAGS=${TK_LIB_SPEC}
    fi
dnl ## Fairly crude hack to find the lib under certain conditions.
    if test "x${TK_LDFLAGS}" = "x" ; then
      tk_libs=`ls ./tk[[7-9]]*.lib ./libtk[[7-9]]*.a 2>/dev/null`
      if test "x${tk_libs}" != "x" ; then
	TK_LDFLAGS="../${tk_libs}"
      fi 
    fi
    if test "x${TK_LDFLAGS}" = "x" ; then
      tk_libs=`ls tcltk/tk[[7-9]]*.lib tcltk/libtk[[7-9]]*.a 2>/dev/null`
      if test "x${tk_libs}" != "x" ; then
	TK_LDFLAGS="../${tk_libs}"
      fi 
    fi
    if test "x${TK_LDFLAGS}" = "x" ; then
      tk_libs=`ls tk/tk[[7-9]]*.lib tk/libtk[[7-9]]*.a 2>/dev/null`
      if test "x${tk_libs}" != "x" ; then
	TK_LDFLAGS="../${tk_libs}"
      fi 
    fi
dnl ## Another attempt to salvage a usable lib.
    if test "x${TK_LDFLAGS}" = "x" ; then
      if test "x${TK_LIB_FILE}" != "x" ; then
        TK_LDFLAGS="`dirname ${TKCONFIG}`/${TK_LIB_FILE}"
      fi
    fi
dnl ## Tack on extra goodies that may be needed.
dnl ## But filter off any directly named static libs.
    if test "x${TK_LIBS}" != "x" ; then
      liblist=""
      for libitem in ${TK_LIBS} ; do
	tmplibitem=`echo "$libitem" | grep -v '\.lib$' | grep -v '\.a$'`
	liblist="$liblist $tmplibitem"
      done
      TK_LDFLAGS="${TK_LDFLAGS} $liblist"
    fi

    AC_SUBST(TK_VERSION)
dnl not actually used, don't export to save symbols
dnl    AC_SUBST(TK_MAJOR_VERSION)
dnl    AC_SUBST(TK_MINOR_VERSION)
    AC_SUBST(TK_DEFS)

dnl not used, don't export to save symbols
dnl    AC_SUBST(TK_LIB_FILE)

dnl not used outside of configure
dnl    AC_SUBST(TK_LIBS)
dnl not used, don't export to save symbols
dnl    AC_SUBST(TK_PREFIX)

dnl not used, don't export to save symbols
dnl    AC_SUBST(TK_EXEC_PREFIX)

    AC_SUBST(TK_BUILD_INCLUDES)
    AC_SUBST(TK_XINCLUDES)
    AC_SUBST(TK_XLIBSW)
    AC_SUBST(TK_BUILD_LIB_SPEC)
    AC_SUBST(TK_LIB_SPEC)

	AC_SUBST(TK_LDFLAGS)
])

# Configure paths for SDL
# Sam Lantinga 9/21/99
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_SDL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for SDL, and define SDL_CFLAGS and SDL_LIBS
dnl
AC_DEFUN(AM_PATH_SDL,
[dnl 
dnl Get the cflags and libraries from the sdl-config script
dnl
AC_ARG_WITH(sdl-prefix,[  --with-sdl-prefix=PFX   Prefix where SDL is installed (optional)],
            sdl_prefix="$withval", sdl_prefix="")
AC_ARG_WITH(sdl-exec-prefix,[  --with-sdl-exec-prefix=PFX Exec prefix where SDL is installed (optional)],
            sdl_exec_prefix="$withval", sdl_exec_prefix="")
AC_ARG_ENABLE(sdltest, [  --disable-sdltest       Do not try to compile and run a test SDL program],
		    , enable_sdltest=yes)

  if test x$sdl_exec_prefix != x ; then
     sdl_args="$sdl_args --exec-prefix=$sdl_exec_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_exec_prefix/bin/sdl-config
     fi
  fi
  if test x$sdl_prefix != x ; then
     sdl_args="$sdl_args --prefix=$sdl_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_prefix/bin/sdl-config
     fi
  fi

  AC_REQUIRE([AC_CANONICAL_TARGET])
  PATH="$prefix/bin:$prefix/usr/bin:$PATH"
  AC_PATH_PROG(SDL_CONFIG, sdl-config, no, [$PATH])
  min_sdl_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for SDL - version >= $min_sdl_version)
  no_sdl=""
  if test "$SDL_CONFIG" = "no" ; then
    no_sdl=yes
  else
    SDL_CFLAGS=`$SDL_CONFIG $sdlconf_args --cflags`
    SDL_LIBS=`$SDL_CONFIG $sdlconf_args --libs`

    sdl_major_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl_minor_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl_micro_version=`$SDL_CONFIG $sdl_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $SDL_CFLAGS"
      CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
      LIBS="$LIBS $SDL_LIBS"
dnl
dnl Now check if the installed SDL is sufficiently new. (Also sanity
dnl checks the results of sdl-config to some extent
dnl
      rm -f conf.sdltest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdltest");
  */
  { FILE *fp = fopen("conf.sdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl_version");
     exit(1);
   }

   if (($sdl_major_version > major) ||
      (($sdl_major_version == major) && ($sdl_minor_version > minor)) ||
      (($sdl_major_version == major) && ($sdl_minor_version == minor) && ($sdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n", $sdl_major_version, $sdl_minor_version, $sdl_micro_version);
      printf("*** of SDL required is %d.%d.%d. If sdl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl-config was wrong, set the environment variable SDL_CONFIG\n");
      printf("*** to point to the correct copy of sdl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_sdl=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdl" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SDL_CONFIG" = "no" ; then
       echo "*** The sdl-config script installed by SDL could not be found"
       echo "*** If SDL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDL_CONFIG environment variable to the"
       echo "*** full path to sdl-config."
     else
       if test -f conf.sdltest ; then
        :
       else
          echo "*** Could not run SDL test program, checking why..."
          CFLAGS="$CFLAGS $SDL_CFLAGS"
          CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
          LIBS="$LIBS $SDL_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include "SDL.h"

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SDL or finding the wrong"
          echo "*** version of SDL. If it is not finding SDL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means SDL was incorrectly installed"
          echo "*** or that you have moved SDL since it was installed. In the latter case, you"
          echo "*** may want to edit the sdl-config script: $SDL_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDL_CFLAGS=""
     SDL_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SDL_CFLAGS)
  AC_SUBST(SDL_LIBS)
  rm -f conf.sdltest
])

# Configure paths for FreeType2
# Marcelo Magallon 2001-10-26, based on gtk.m4 by Owen Taylor

dnl AC_CHECK_FT2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for FreeType2, and define FT2_CFLAGS and FT2_LIBS
dnl
AC_DEFUN(AC_CHECK_FT2,
[dnl
dnl Get the cflags and libraries from the freetype-config script
dnl
AC_ARG_WITH(ft-prefix,
[  --with-ft-prefix=PREFIX
                          Prefix where FreeType is installed (optional)],
            ft_config_prefix="$withval", ft_config_prefix="")
AC_ARG_WITH(ft-exec-prefix,
[  --with-ft-exec-prefix=PREFIX
                          Exec prefix where FreeType is installed (optional)],
            ft_config_exec_prefix="$withval", ft_config_exec_prefix="")
AC_ARG_ENABLE(freetypetest,
[  --disable-freetypetest  Do not try to compile and run
                          a test FreeType program],
[case "${enableval}" in 
yes) enable_fttest=yes ;;
no) enable_fttest=no ;;
*) AC_MSG_ERROR(bad value ${enableval} for freetypetest option) ;;
esac], enable_fttest=yes)

if test x$ft_config_exec_prefix != x ; then
  ft_config_args="$ft_config_args --exec-prefix=$ft_config_exec_prefix"
  if test x${FT2_CONFIG+set} != xset ; then
    FT2_CONFIG=$ft_config_exec_prefix/bin/freetype-config
  fi
fi
if test x$ft_config_prefix != x ; then
  ft_config_args="$ft_config_args --prefix=$ft_config_prefix"
  if test x${FT2_CONFIG+set} != xset ; then
    FT2_CONFIG=$ft_config_prefix/bin/freetype-config
  fi
fi
AC_PATH_PROG(FT2_CONFIG, freetype-config, no)

min_ft_version=ifelse([$1], ,6.1.0,$1)
AC_MSG_CHECKING(for FreeType - version >= $min_ft_version)
no_ft=""
if test "$FT2_CONFIG" = "no" ; then
  no_ft=yes
else
  FT2_CFLAGS=`$FT2_CONFIG $ft_config_args --cflags`
  FT2_LIBS=`$FT2_CONFIG $ft_config_args --libs`
  ft_config_major_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  ft_config_minor_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  ft_config_micro_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  ft_min_major_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  ft_min_minor_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  ft_min_micro_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  if test x$enable_fttest = xyes ; then
    ft_config_is_lt=""
    if test $ft_config_major_version -lt $ft_min_major_version ; then
      ft_config_is_lt=yes
    else
      if test $ft_config_major_version -eq $ft_min_major_version ; then
        if test $ft_config_minor_version -lt $ft_min_minor_version ; then
          ft_config_is_lt=yes
        else
          if test $ft_config_minor_version -eq $ft_min_minor_version ; then
            if test $ft_config_micro_version -lt $ft_min_micro_version ; then
              ft_config_is_lt=yes
            fi
          fi
        fi
      fi
    fi
    if test x$ft_config_is_lt = xyes ; then
      no_ft=yes
    else
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $FT2_CFLAGS"
      LIBS="$FT2_LIBS $LIBS"
dnl
dnl Sanity checks for the results of freetype-config to some extent
dnl
      AC_TRY_RUN([
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>

int
main()
{
  FT_Library library;
  FT_Error error;

  error = FT_Init_FreeType(&library);

  if (error)
    return 1;
  else
  {
    FT_Done_FreeType(library);
    return 0;
  }
}
],, no_ft=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
      CFLAGS="$ac_save_CFLAGS"
      LIBS="$ac_save_LIBS"
    fi             # test $ft_config_version -lt $ft_min_version
  fi               # test x$enable_fttest = xyes
fi                 # test "$FT2_CONFIG" = "no"
if test x$no_ft = x ; then
   AC_MSG_RESULT(yes)
   ifelse([$2], , :, [$2])
else
   AC_MSG_RESULT(no)
   if test "$FT2_CONFIG" = "no" ; then
     echo "*** The freetype-config script installed by FreeType 2 could not be found."
     echo "*** If FreeType 2 was installed in PREFIX, make sure PREFIX/bin is in"
     echo "*** your path, or set the FT2_CONFIG environment variable to the"
     echo "*** full path to freetype-config."
   else
     if test x$ft_config_is_lt = xyes ; then
       echo "*** Your installed version of the FreeType 2 library is too old."
       echo "*** If you have different versions of FreeType 2, make sure that"
       echo "*** correct values for --with-ft-prefix or --with-ft-exec-prefix"
       echo "*** are used, or set the FT2_CONFIG environment variable to the"
       echo "*** full path to freetype-config."
     else
       echo "*** The FreeType test program failed to run.  If your system uses"
       echo "*** shared libraries and they are installed outside the normal"
       echo "*** system library path, make sure the variable LD_LIBRARY_PATH"
       echo "*** (or whatever is appropiate for your system) is correctly set."
     fi
   fi
   FT2_CFLAGS=""
   FT2_LIBS=""
   ifelse([$3], , :, [$3])
fi
AC_SUBST(FT2_CFLAGS)
AC_SUBST(FT2_LIBS)
])

# Configure paths for ParaGUI
# Alexander Pipelka 17.05.2000
# stolen from Sam Lantinga 
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_PARAGUI([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for PARAGUI, and define PARAGUI_CFLAGS and PARAGUI_LIBS
dnl
AC_DEFUN(AM_PATH_PARAGUI,
[dnl 
dnl Get the cflags and libraries from the paragui-config script
dnl
AC_ARG_WITH(paragui-prefix,[  --with-paragui-prefix=PFX   Prefix where PARAGUI is installed (optional)],
            paragui_prefix="$withval", paragui_prefix="")
AC_ARG_WITH(paragui-exec-prefix,[  --with-paragui-exec-prefix=PFX Exec prefix where PARAGUI is installed (optional)],
            paragui_exec_prefix="$withval", paragui_exec_prefix="")
AC_ARG_ENABLE(paraguitest, [  --disable-paraguitest       Do not try to compile and run a test PARAGUI program],
[case "${enableval}" in
yes) enable_paraguitest=yes ;;
no) enable_paraguitest=no ;;
*) AC_MSG_ERROR(bad value ${enableval} for paraguitest option) ;;
esac], enable_paraguitest=yes)

  if test x$paragui_exec_prefix != x ; then
     paragui_args="$paragui_args --exec-prefix=$paragui_exec_prefix"
     if test x${PARAGUI_CONFIG+set} != xset ; then
        PARAGUI_CONFIG=$paragui_exec_prefix/bin/paragui-config
     fi
  fi
  if test x$paragui_prefix != x ; then
     paragui_args="$paragui_args --prefix=$paragui_prefix"
     if test x${PARAGUI_CONFIG+set} != xset ; then
        PARAGUI_CONFIG=$paragui_prefix/bin/paragui-config
     fi
  fi

  AC_PATH_PROG(PARAGUI_CONFIG, paragui-config, no, [$prefix/usr/bin:$prefix/bin:$PATH])
  min_paragui_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for PARAGUI - version >= $min_paragui_version)
  no_paragui=""
  if test "$PARAGUI_CONFIG" = "no" ; then
    no_paragui=yes
  else
    PARAGUI_CFLAGS=`$PARAGUI_CONFIG $paraguiconf_args --cflags`
    PARAGUI_LIBS=`$PARAGUI_CONFIG $paraguiconf_args --libs`

    paragui_major_version=`$PARAGUI_CONFIG $paragui_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    paragui_minor_version=`$PARAGUI_CONFIG $paragui_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    paragui_micro_version=`$PARAGUI_CONFIG $paragui_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_paraguitest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $PARAGUI_CFLAGS"
      LIBS="$LIBS $PARAGUI_LIBS"
dnl
dnl Now check if the installed PARAGUI is sufficiently new. (Also sanity
dnl checks the results of paragui-config to some extent
dnl
      rm -f conf.paraguitest
      AC_LANG_CPLUSPLUS
      AC_TRY_RUN([
#include "paragui.h"
#include "pgapplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  PG_Application app;
  
  /* This hangs on some systems (?)
  system ("touch conf.paraguitest");
  */
  { FILE *fp = fopen("conf.paraguitest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_paragui_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_paragui_version");
     exit(1);
   }

   if (($paragui_major_version > major) ||
      (($paragui_major_version == major) && ($paragui_minor_version > minor)) ||
      (($paragui_major_version == major) && ($paragui_minor_version == minor) && ($paragui_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'paragui-config --version' returned %d.%d.%d, but the minimum version\n", $paragui_major_version, $paragui_minor_version, $paragui_micro_version);
      printf("*** of PARAGUI required is %d.%d.%d. If paragui-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If paragui-config was wrong, set the environment variable PARAGUI_CONFIG\n");
      printf("*** to point to the correct copy of paragui-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_paragui=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_paragui" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$PARAGUI_CONFIG" = "no" ; then
       echo "*** The paragui-config script installed by PARAGUI could not be found"
       echo "*** If PARAGUI was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the PARAGUI_CONFIG environment variable to the"
       echo "*** full path to paragui-config."
     else
       if test -f conf.paraguitest ; then
        :
       else
          echo "*** Could not run PARAGUI test program, checking why..."
          CXXFLAGS="$CXXFLAGS $PARAGUI_CFLAGS"
          LIBS="$LIBS $PARAGUI_LIBS"
          AC_TRY_LINK([
#include "paragui.h"
#include <stdio.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding PARAGUI or finding the wrong"
          echo "*** version of PARAGUI. If it is not finding PARAGUI, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means PARAGUI was incorrectly installed"
          echo "*** or that you have moved PARAGUI since it was installed. In the latter case, you"
          echo "*** may want to edit the paragui-config script: $PARAGUI_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     PARAGUI_CFLAGS=""
     PARAGUI_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(PARAGUI_CFLAGS)
  AC_SUBST(PARAGUI_LIBS)
  rm -f conf.paraguitest
])
