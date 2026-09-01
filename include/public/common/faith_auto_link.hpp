 /*
  *   DESCRIPTION: Automatic library inclusion for Microsoft compilers.
  */

/*************************************************************************

USAGE:
~~~~~~

Before including this header you must define one or more of define the following macros:

FAITH_LIB_NAME:           Required: A string containing the basename of the library,
                          for example net.
FAITH_LIB_DIAGNOSTIC:     Optional: when set the header will print out the name
                          of the library selected (useful for debugging).

These macros will be undef'ed at the end of the header, further this header
has no include guards - so be sure to include it only once from your library!

Algorithm:
~~~~~~~~~~

Libraries for Microsoft compilers are automatically
selected here, the name of the lib is selected according to the following
formula:

FAITH_LIB_NAME
   + "_"
   + FAITH_LIB_RTL_OPT
   + "_"
   + FAITH_LIB_DEBUG_OPT
   + ".lib"

These are defined as:

FAITH_LIB_NAME:       The base name of the lib ( for example FAITH_regex).

FAITH_LIB_DEBUG_OPT: "debug" for debug version,otherwise "release".

FAITH_LIB_RTL_OPT:   for c/cpp run-time library

***************************************************************************/

#ifndef FAITH_LIB_NAME
#  error "Macro FAITH_LIB_NAME not set (internal error)"
#endif

// check ths supported complilers

// set FAITH_LIB_RTL_OPT for c/cpp run-time library
#if defined(_MT) && defined(_DLL)
#  define FAITH_LIB_RTL_OPT "dynamic-rtl"
#else
#	if defined(_MT)
#		define FAITH_LIB_RTL_OPT "static-rtl"
#	else
#		define FAITH_LIB_RTL_OPT "static-rtl"
#	endif
#endif

// set FAITH_LIB_DEBUG_OPT
#ifdef _DEBUG
#  define FAITH_LIB_DEBUG_OPT "debug"
#else
#  define FAITH_LIB_DEBUG_OPT "release"
#endif

// now include the lib:
#if defined(FAITH_LIB_NAME) \
      && defined(FAITH_LIB_RTL_OPT) \
      && defined(FAITH_LIB_DEBUG_OPT)
#else
#  error "some required macros where not defined (internal logic error)."
#endif

#if defined _MSC_VER	
#	 pragma comment(lib, FAITH_LIB_NAME "_" FAITH_LIB_RTL_OPT "_" FAITH_LIB_DEBUG_OPT ".lib")
#elif defined __GNUC__
//#	import <lzo2_static-rtl_release.lib>
#else
#	 error "Unknown compiler, only support for msvc and gcc."
#endif

#  ifdef FAITH_LIB_DIAGNOSTIC
#	pragma message ("Linking to lib file: " FAITH_LIB_NAME "_" FAITH_LIB_RTL_OPT "_" FAITH_LIB_DEBUG_OPT ".lib")
#endif

// finally undef any macros we may have set:
#if defined(FAITH_LIB_NAME)
#  undef FAITH_LIB_NAME
#endif
#if defined(FAITH_LIB_RTL_OPT)
#  undef FAITH_LIB_RTL_OPT
#endif
#if defined(FAITH_LIB_DEBUG_OPT)
#  undef FAITH_LIB_DEBUG_OPT
#endif
