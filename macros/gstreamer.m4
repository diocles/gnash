dnl  
dnl    Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl  
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

dnl Linking Gnash statically or dynamically with other modules is making a
dnl combined work based on Gnash. Thus, the terms and conditions of the GNU
dnl General Public License cover the whole combination.
dnl
dnl As a special exception, the copyright holders of Gnash give you
dnl permission to combine Gnash with free software programs or libraries
dnl that are released under the GNU LGPL and with code included in any
dnl release of Talkback distributed by the Mozilla Foundation. You may
dnl copy and distribute such a system following the terms of the GNU GPL
dnl for all but the LGPL-covered parts and Talkback, and following the
dnl LGPL for the LGPL-covered parts.
dnl
dnl Note that people who make modified versions of Gnash are not obligated
dnl to grant this special exception for their modified versions; it is their
dnl choice whether to do so. The GNU General Public License gives permission
dnl to release a modified version without this exception; this exception
dnl also makes it possible to release a modified version which carries
dnl forward this exception.
dnl  
dnl 

AC_DEFUN([GNASH_PATH_GSTREAMER],
[
  dnl Look for the header
  AC_ARG_WITH(gst_incl, [  --with-gst-incl        directory where libgstreamer header is], with_gstreamer_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_gstreamer_incl,[
    if test x"${with_gstreamer_incl}" != x ; then
      if test -f ${with_gstreamer_incl}/gst/gst.h ; then
	ac_cv_path_gstreamer_incl=-I`(cd ${with_gstreamer_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_gstreamer_incl} directory doesn't contain gstreamer/gstreamergl.h])
      fi
    fi])

    dnl Attempt to find the top level directory, which unfortunately has a
    dnl version number attached. At least on Debain based systems, this
    dnl doesn't seem to get a directory that is unversioned.
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gstreamer_incl}" = x; then
      ac_cv_path_gstreamer_incl=`$PKG_CONFIG --cflags gstreamer-0.10`
      dnl The following is actually unchecked
    fi

    if test x"${ac_cv_path_gstreamer_incl}" = x; then
      AC_MSG_CHECKING([for the Gstreamer Version])
      pathlist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

      gnash_gstreamer_topdir=""
      gnash_gstreamer_version=""
      for i in $pathlist; do
	for j in `ls -dr $i/gstreamer-[[0-9]].[[0-9]][[0-9]] 2>/dev/null`; do
 	  if test -f $j/gst/gst.h; then
	    gnash_gstreamer_topdir=`basename $j`
	    gnash_gstreamer_version=`echo ${gnash_gstreamer_topdir} | sed -e 's:gstreamer-::'`
	    break
 	  fi
	done
      done
    fi

    if test x"${gnash_gstreamer_topdir}" = x; then
      AC_MSG_RESULT(none)
    else
      AC_MSG_RESULT([${gnash_gstreamer_version}])
    fi

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_gstreamer_incl}" = x; then
      AC_CHECK_HEADERS(gst/gst.h, [ac_cv_path_gstreamer_incl=""],[
      if test x"${ac_cv_path_gstreamer_incl}" = x; then
        AC_MSG_CHECKING([for libgstreamer header])
        incllist="/sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/gstr/gst.h; then
            ac_cv_path_gstreamer_incl="-I$i"
	    break;
	  else
	    if test -f $i/${gnash_gstreamer_topdir}/gst/gst.h; then
	      ac_cv_path_gstreamer_incl="-I$i/${gnash_gstreamer_topdir}"
	      break
	    fi
	  fi
        done
      fi])
    fi

     if test x"${ac_cv_path_gstreamer_incl}" != x ; then
       AC_MSG_RESULT(yes)
     else
       AC_MSG_RESULT(no)
     fi

      dnl Look for the library
    AC_ARG_WITH(gst_lib, [  --with-gst-lib         directory where gstreamer library is], with_gstreamer_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_gstreamer_lib,[
      if test x"${with_gstreamer_lib}" != x ; then
        if test -f ${with_gstreamer_lib}/libgstreamer-0.10.so; then
	  ac_cv_path_gstreamer_lib=`(cd ${with_gstreamer_lib}; pwd)`
        else
	  AC_MSG_ERROR([${with_gstreamer_lib} directory doesn't contain libgstreamer-0.10.so.])
        fi
      fi
      ])

    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gstreamer_lib}" = x; then
      ac_cv_path_gstreamer_lib=`$PKG_CONFIG --libs gstreamer-0.10`
    fi

dnl If the header doesn't exist, there is no point looking for
dnl the library. 
      if test x"${ac_cv_path_gstreamer_incl}" != x; then
        AC_MSG_CHECKING([for libgstreamer library])
        AC_CHECK_LIB(gstreamer-${gnash_gstreamer_version}, gst_plugin_init, [ac_cv_path_gstreamer_lib="-lgstreamer-${gnash_gstreamer_version}"],[
          libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/gstreamer-${gnash_gstreamer_version}/libgstreamer-0.10.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_gstreamer_lib="-L$i -lgstreamer-${gnash_gstreamer_version}"
	        break
              else
	        ac_cv_path_gstreamer_lib="-lgstreamer-${gnash_gstreamer_version}"
	        break
	      fi
	    else
	      if test -f $i/libgstreamer-${gnash_gstreamer_version}.a -o -f $i/gstreamer-${gnash_gstreamer_version}.so; then
		ac_cv_path_gstreamer_lib="-L$i/${gnash_gstreamer_topdir} -lgstreamer-${gnash_gstreamer_version}"
		break
	      fi
	    fi
          done])
      else
	if test -f $i/gstreamer-${gnash_gstreamer_version}/libgstreamer-0.10.so; then
          if test x"${ac_cv_path_gstreamer_lib}" != x"/usr/lib"; then
	    ac_cv_path_gstreamer_lib="-L${ac_cv_path_gstreamer_lib} -lgstreamer-${gnash_gstreamer_version}"
           else
	    ac_cv_path_gstreamer_lib="-lgstreamer-${gnash_gstreamer_version}"
          fi
        fi
      fi

  if test x"${ac_cv_path_gstreamer_incl}" != x ; then
    GSTREAMER_CFLAGS="${ac_cv_path_gstreamer_incl}"
  else
    GSTREAMER_CFLAGS=""
  fi

  dnl
  dnl Call AC_CHECK_HEADERS again here to
  dnl get HAVE_* macros automatically defined
  dnl
  dnl NOTE: we need additional CFLAGS for things to work
  dnl       (stuff included by gstreamer header)
  dnl

  ac_save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $GSTREAMER_CFLAGS $GLIB_CFLAGS $LIBXML_CFLAGS"
  echo "CFLAGS==$CFLAGS"
  AC_CHECK_HEADERS(gst/gst.h)
  CFLAGS="$ac_save_CFLAGS"

  if test x"${ac_cv_path_gstreamer_lib}" != x ; then
    GSTREAMER_LIBS="${ac_cv_path_gstreamer_lib}"
dnl    AC_MSG_WARN([You need GStreamer 0.10 or greater for sound support!])
  else
    GSTREAMER_LIBS=""
  fi

  AC_SUBST(GSTREAMER_CFLAGS)
  AC_SUBST(GSTREAMER_LIBS)
])
