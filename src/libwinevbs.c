#include "libwinevbs.h"

static libwinevbs_callbacks_t g_callbacks = { 0 };

void libwinevbs_init(const libwinevbs_callbacks_t* callbacks)
{
   if (callbacks)
      g_callbacks = *callbacks;
}

void libwinevbs_shutdown(void)
{
   memset(&g_callbacks, 0, sizeof(g_callbacks));
}

void external_log(libwinevbs_log_level_t level, const char* format, ...)
{
   if (g_callbacks.log) {
      va_list args;
      va_start(args, format);
      g_callbacks.log(level, format, args);
      va_end(args);
   }
}

HRESULT external_create_object(const WCHAR* progid, IClassFactory* cf, IUnknown** obj)
{
   if (g_callbacks.create_object)
      return g_callbacks.create_object(progid, cf, obj);

   external_log(LIBWINEVBS_LOG_ERROR, "Creating an object is not implemented");
   return CLASS_E_CLASSNOTAVAILABLE;
}
