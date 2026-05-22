#include "libwinevbs.h"
#include "olectl.h"

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

void libwinevbs_log(libwinevbs_log_level_t level, const char* format, ...)
{
   if (g_callbacks.log) {
      va_list args;
      va_start(args, format);
      g_callbacks.log(level, format, args);
      va_end(args);
   }
}

HRESULT libwinevbs_create_object(const WCHAR* progid, IClassFactory* cf, IUnknown** obj)
{
   if (g_callbacks.create_object)
      return g_callbacks.create_object(progid, cf, obj);

   return CLASS_E_CLASSNOTAVAILABLE;
}

const char* libwinevbs_hresult_name(HRESULT hr)
{
   switch (hr) {
      /* generic */
      case S_OK:                       return "S_OK";
      case S_FALSE:                    return "S_FALSE";
      case E_NOTIMPL:                  return "E_NOTIMPL";
      case E_NOINTERFACE:              return "E_NOINTERFACE";
      case E_POINTER:                  return "E_POINTER";
      case E_ABORT:                    return "E_ABORT";
      case E_FAIL:                     return "E_FAIL";
      case E_UNEXPECTED:               return "E_UNEXPECTED";
      case E_ACCESSDENIED:             return "E_ACCESSDENIED";
      case E_HANDLE:                   return "E_HANDLE";
      case E_OUTOFMEMORY:              return "E_OUTOFMEMORY";
      case E_INVALIDARG:               return "E_INVALIDARG";
      case CLASS_E_CLASSNOTAVAILABLE:  return "CLASS_E_CLASSNOTAVAILABLE";

      /* IDispatch::Invoke / GetIDsOfNames */
      case DISP_E_UNKNOWNINTERFACE:    return "DISP_E_UNKNOWNINTERFACE";
      case DISP_E_MEMBERNOTFOUND:      return "DISP_E_MEMBERNOTFOUND";
      case DISP_E_PARAMNOTFOUND:       return "DISP_E_PARAMNOTFOUND";
      case DISP_E_TYPEMISMATCH:        return "DISP_E_TYPEMISMATCH";
      case DISP_E_UNKNOWNNAME:         return "DISP_E_UNKNOWNNAME";
      case DISP_E_NONAMEDARGS:         return "DISP_E_NONAMEDARGS";
      case DISP_E_BADVARTYPE:          return "DISP_E_BADVARTYPE";
      case DISP_E_EXCEPTION:           return "DISP_E_EXCEPTION";
      case DISP_E_OVERFLOW:            return "DISP_E_OVERFLOW";
      case DISP_E_BADINDEX:            return "DISP_E_BADINDEX";
      case DISP_E_UNKNOWNLCID:         return "DISP_E_UNKNOWNLCID";
      case DISP_E_ARRAYISLOCKED:       return "DISP_E_ARRAYISLOCKED";
      case DISP_E_BADPARAMCOUNT:       return "DISP_E_BADPARAMCOUNT";
      case DISP_E_PARAMNOTOPTIONAL:    return "DISP_E_PARAMNOTOPTIONAL";
      case DISP_E_NOTACOLLECTION:      return "DISP_E_NOTACOLLECTION";
      case DISP_E_DIVBYZERO:           return "DISP_E_DIVBYZERO";
      case DISP_E_BUFFERTOOSMALL:      return "DISP_E_BUFFERTOOSMALL";

      /* VBScript control runtime errors (olectl.h) */
      case CTL_E_ILLEGALFUNCTIONCALL:  return "CTL_E_ILLEGALFUNCTIONCALL";
      case CTL_E_OVERFLOW:             return "CTL_E_OVERFLOW";
      case CTL_E_OUTOFMEMORY:          return "CTL_E_OUTOFMEMORY";
      case CTL_E_DIVISIONBYZERO:       return "CTL_E_DIVISIONBYZERO";
      case CTL_E_FILENOTFOUND:         return "CTL_E_FILENOTFOUND";
      case CTL_E_DEVICEIOERROR:        return "CTL_E_DEVICEIOERROR";
      case CTL_E_FILEALREADYEXISTS:    return "CTL_E_FILEALREADYEXISTS";
      case CTL_E_PERMISSIONDENIED:     return "CTL_E_PERMISSIONDENIED";
      case CTL_E_PATHFILEACCESSERROR:  return "CTL_E_PATHFILEACCESSERROR";
      case CTL_E_PATHNOTFOUND:         return "CTL_E_PATHNOTFOUND";

      /* scrrun Scripting.Dictionary errors (scrrun.h) */
      case STD_CTL_SCODE(457):         return "CTL_E_KEY_ALREADY_EXISTS";
      case STD_CTL_SCODE(32811):       return "CTL_E_ELEMENT_NOT_FOUND";

      default:                         return "?";
   }
}
