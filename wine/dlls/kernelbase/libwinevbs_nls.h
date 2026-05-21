#ifndef __LIBWINEVBS_NLS_H
#define __LIBWINEVBS_NLS_H

#include "windef.h"
#include "winnls.h"

int libwinevbs_get_locale_info( LCID lcid, LCTYPE type, WCHAR *buffer, int len );

#endif
