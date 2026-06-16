#ifndef LIBWINEVBS_H
#define LIBWINEVBS_H

#define LIBWINEVBS_VERSION_MAJOR 0
#define LIBWINEVBS_VERSION_MINOR 8
#define LIBWINEVBS_VERSION_PATCH 0

#define _LIBWINEVBS_STR(x) #x
#define LIBWINEVBS_STR(x) _LIBWINEVBS_STR(x)

#define LIBWINEVBS_VERSION LIBWINEVBS_STR(LIBWINEVBS_VERSION_MAJOR) "." LIBWINEVBS_STR(LIBWINEVBS_VERSION_MINOR) "." LIBWINEVBS_STR(LIBWINEVBS_VERSION_PATCH)
#define LIBWINEVBS_MINOR_VERSION LIBWINEVBS_STR(LIBWINEVBS_VERSION_MAJOR) "." LIBWINEVBS_STR(LIBWINEVBS_VERSION_MINOR)

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "ole2.h"
#include "activscp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
   #ifdef LIBWINEVBS_EXPORT
      #define LIBWINEVBS_API __declspec(dllexport)
   #else
      #define LIBWINEVBS_API __declspec(dllimport)
   #endif
#else
   #ifdef LIBWINEVBS_EXPORT
      #define LIBWINEVBS_API __attribute__((visibility("default")))
   #else
      #define LIBWINEVBS_API
   #endif
#endif

typedef enum {
   LIBWINEVBS_LOG_INFO,
   LIBWINEVBS_LOG_DEBUG,
   LIBWINEVBS_LOG_WARN,
   LIBWINEVBS_LOG_ERROR
} libwinevbs_log_level_t;

LIBWINEVBS_API void libwinevbs_log(libwinevbs_log_level_t level, const char* format, ...);

/* Returns the symbolic name for a known HRESULT (e.g. "DISP_E_MEMBERNOTFOUND"),
   or "?" when the value is not recognized. For diagnostic logging only. */
LIBWINEVBS_API const char* libwinevbs_hresult_name(HRESULT hr);

typedef struct {
   void (*log)(libwinevbs_log_level_t level, const char* format, va_list args);
   HRESULT (*create_object)(const WCHAR* progid, IClassFactory* cf, IUnknown** obj);
} libwinevbs_callbacks_t;

LIBWINEVBS_API void libwinevbs_init(const libwinevbs_callbacks_t* callbacks);
LIBWINEVBS_API void libwinevbs_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
