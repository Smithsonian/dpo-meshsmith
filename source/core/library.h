#ifndef INTERMESH_ENGINE_LIBRARY_H
#define INTERMESH_ENGINE_LIBRARY_H

#include "core/setup.h"

#if defined(WIN32)
#define F_DECLARE_EXPORT __declspec(dllexport)
#define F_DECLARE_IMPORT __declspec(dllimport)
#else
#define F_DECLARE_EXPORT
#define F_DECLARE_IMPORT
#endif

#ifndef INTERMESH_ENGINE_SHARED
#  define INTERMESH_ENGINE_EXPORT
#elif defined(INTERMESH_ENGINE_LIB)
#  define INTERMESH_ENGINE_EXPORT F_DECLARE_EXPORT
#else
#  define INTERMESH_ENGINE_EXPORT F_DECLARE_IMPORT
#endif

#endif // INTERMESH_ENGINE_LIBRARY_H