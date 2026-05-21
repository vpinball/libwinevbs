#include <string.h>
#include <wchar.h>

#include "windef.h"
#include "winbase.h"
#include "winnls.h"
#include "libwinevbs_nls.h"

static const WCHAR *months[12] =
{
    L"January", L"February", L"March", L"April", L"May", L"June",
    L"July", L"August", L"September", L"October", L"November", L"December"
};
static const WCHAR *abbrev_months[12] =
{
    L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"
};
static const WCHAR *days[7] =
{
    L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday", L"Sunday"
};
static const WCHAR *abbrev_days[7] =
{
    L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun"
};

static int ret_str( const WCHAR *data, LCTYPE type, WCHAR *buffer, int len )
{
    int datalen = wcslen( data ) + 1;

    if (type & LOCALE_RETURN_NUMBER)
    {
        SetLastError( ERROR_INVALID_FLAGS );
        return 0;
    }
    if (!len) return datalen;
    if (datalen > len)
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return 0;
    }
    memcpy( buffer, data, datalen * sizeof(WCHAR) );
    return datalen;
}

static int ret_num( UINT val, LCTYPE type, WCHAR *buffer, int len )
{
    WCHAR tmp[16];

    if (type & LOCALE_RETURN_NUMBER)
    {
        int ret = sizeof(UINT) / sizeof(WCHAR);

        if (!len) return ret;
        if (ret > len)
        {
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return 0;
        }
        memcpy( buffer, &val, sizeof(val) );
        return ret;
    }
    swprintf( tmp, ARRAY_SIZE(tmp), L"%u", val );
    return ret_str( tmp, type, buffer, len );
}

int libwinevbs_get_locale_info( LCID lcid, LCTYPE type, WCHAR *buffer, int len )
{
    LCTYPE lc = LOWORD(type);

    if (lc >= LOCALE_SMONTHNAME1 && lc <= LOCALE_SMONTHNAME12)
        return ret_str( months[lc - LOCALE_SMONTHNAME1], type, buffer, len );
    if (lc >= LOCALE_SABBREVMONTHNAME1 && lc <= LOCALE_SABBREVMONTHNAME12)
        return ret_str( abbrev_months[lc - LOCALE_SABBREVMONTHNAME1], type, buffer, len );
    if (lc >= LOCALE_SDAYNAME1 && lc <= LOCALE_SDAYNAME7)
        return ret_str( days[lc - LOCALE_SDAYNAME1], type, buffer, len );
    if (lc >= LOCALE_SABBREVDAYNAME1 && lc <= LOCALE_SABBREVDAYNAME7)
        return ret_str( abbrev_days[lc - LOCALE_SABBREVDAYNAME1], type, buffer, len );

    switch (lc)
    {
    case LOCALE_SDECIMAL:         return ret_str( L".", type, buffer, len );
    case LOCALE_STHOUSAND:        return ret_str( L",", type, buffer, len );
    case LOCALE_SGROUPING:        return ret_str( L"3;0", type, buffer, len );
    case LOCALE_SNEGATIVESIGN:    return ret_str( L"-", type, buffer, len );
    case LOCALE_IDIGITS:          return ret_num( 2, type, buffer, len );
    case LOCALE_ILZERO:           return ret_num( 1, type, buffer, len );
    case LOCALE_INEGNUMBER:       return ret_num( 1, type, buffer, len );
    case LOCALE_SCURRENCY:        return ret_str( L"$", type, buffer, len );
    case LOCALE_SMONDECIMALSEP:   return ret_str( L".", type, buffer, len );
    case LOCALE_SMONTHOUSANDSEP:  return ret_str( L",", type, buffer, len );
    case LOCALE_SMONGROUPING:     return ret_str( L"3;0", type, buffer, len );
    case LOCALE_ICURRDIGITS:
    case LOCALE_IINTLCURRDIGITS:  return ret_num( 2, type, buffer, len );
    case LOCALE_ICURRENCY:        return ret_num( 0, type, buffer, len );
    case LOCALE_INEGCURR:         return ret_num( 0, type, buffer, len );
    case LOCALE_IDATE:            return ret_num( 0, type, buffer, len );
    case LOCALE_SDATE:            return ret_str( L"/", type, buffer, len );
    case LOCALE_SSHORTDATE:       return ret_str( L"M/d/yyyy", type, buffer, len );
    case LOCALE_SLONGDATE:        return ret_str( L"dddd, MMMM d, yyyy", type, buffer, len );
    case LOCALE_SYEARMONTH:       return ret_str( L"MMMM yyyy", type, buffer, len );
    case LOCALE_SMONTHDAY:        return ret_str( L"MMMM d", type, buffer, len );
    case LOCALE_STIMEFORMAT:      return ret_str( L"h:mm:ss tt", type, buffer, len );
    case LOCALE_STIME:            return ret_str( L":", type, buffer, len );
    case LOCALE_ITIME:            return ret_num( 0, type, buffer, len );
    case LOCALE_ITLZERO:          return ret_num( 0, type, buffer, len );
    case LOCALE_S1159:            return ret_str( L"AM", type, buffer, len );
    case LOCALE_S2359:            return ret_str( L"PM", type, buffer, len );
    case LOCALE_IFIRSTDAYOFWEEK:  return ret_num( 6, type, buffer, len );
    case LOCALE_IFIRSTWEEKOFYEAR: return ret_num( 0, type, buffer, len );
    default:                      return ret_str( L"", type, buffer, len );
    }
}
