/* include/psconfig.h.  Generated from psconfig.h.in by configure.  */
/* include/psconfig.h.in.  Generated from configure.ac by autoheader.  */


#ifdef __CONFIG_H__
#   error PLEASE include this file only once in each .cpp file! (not in .h)
#endif
#define __CONFIG_H__

/* we'll use CS everywhere... and that too has it's config.h like type... we
 * just include it here
 */
#include <cssysdef.h>


/* Define when compiling for MacOS/X */
/* #undef CS_PLATFORM_MACOSX */

/* Define when compiling for Unix and Unix-like (i.e. Mac OS X) */
#define CS_PLATFORM_UNIX /**/

/* Define when compiling for Win32 */
/* #undef CS_PLATFORM_WIN32 */

/* Define if building universal binaries on MacOSX. */
/* #undef CS_UNIVERSAL_BINARY */

/* Define to 1 if you have the <elf.h> header file. */
#define HAVE_ELF_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "Eulora"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Eulora 0.0.0.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "eulora"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.0.0.1"

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 8

/* The number of bytes in type void* */
/* #undef SIZEOF_VOIDP */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define when using normal sockets */
#define USE_UNISOCK /**/

/* Define when using winsock */
/* #undef USE_WINSOCK */

/* Avoid problem caused by missing <Carbon/CarbonSound.h> */
/* #undef __CARBONSOUND__ */

/* define socklen_t if sys headers don't do that */
/* #undef socklen_t */
