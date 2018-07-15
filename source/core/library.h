#ifndef _MESHSMITH_LIBRARY_H
#define _MESHSMITH_LIBRARY_H

#include "core/setup.h"

#if defined(WIN32)
#define F_DECLARE_EXPORT __declspec(dllexport)
#define F_DECLARE_IMPORT __declspec(dllimport)
#else
#define F_DECLARE_EXPORT
#define F_DECLARE_IMPORT
#endif

#ifndef MESHSMITH_CORE_SHARED
#  define MESHSMITH_CORE_EXPORT
#elif defined(MESHSMITH_CORE_LIB)
#  define MESHSMITH_CORE_EXPORT F_DECLARE_EXPORT
#else
#  define MESHSMITH_CORE_EXPORT F_DECLARE_IMPORT
#endif

#endif // _MESHSMITH_LIBRARY_H