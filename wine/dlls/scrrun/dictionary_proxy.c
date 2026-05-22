#include "libwinevbs.h"

static HRESULT WINAPI dictionary_GetIDsOfNames(IDictionary *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Add", 0x00000001 },
			{ L"CompareMode", 0x00000009 },
			{ L"Count", 0x00000002 },
			{ L"Exists", 0x00000003 },
			{ L"HashVal", 0x0000000a },
			{ L"Item", DISPID_VALUE },
			{ L"Items", 0x00000004 },
			{ L"Key", 0x00000005 },
			{ L"Keys", 0x00000006 },
			{ L"Remove", 0x00000007 },
			{ L"RemoveAll", 0x00000008 }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static const char *dictionary_dispid_name(DISPID dispId)
{
	switch(dispId) {
		case DISPID_VALUE: return "Item";
		case 0x00000001: return "Add";
		case 0x00000002: return "Count";
		case 0x00000003: return "Exists";
		case 0x00000004: return "Items";
		case 0x00000005: return "Key";
		case 0x00000006: return "Keys";
		case 0x00000007: return "Remove";
		case 0x00000008: return "RemoveAll";
		case 0x00000009: return "CompareMode";
		case DISPID_NEWENUM: return "_NewEnum";
		case 0x0000000a: return "HashVal";
		default: return "?";
	}
}

static HRESULT WINAPI dictionary_Invoke(IDictionary *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYPUTREF) {
				// line 108: [id(DISPID_VALUE), propputref]HRESULT Item([in] VARIANT* Key, [in] VARIANT* pRetItem);
				if (pDispParams->cArgs < 2) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_putref_Item(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 111: [id(DISPID_VALUE), propput]HRESULT Item([in] VARIANT* Key, [in] VARIANT* pRetItem);
				if (pDispParams->cArgs < 2) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_put_Item(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			else if (wFlags & DISPATCH_PROPERTYGET) {
				// line 114: [id(DISPID_VALUE), propget]HRESULT Item([in] VARIANT* Key, [out, retval] VARIANT* pRetItem);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = dictionary_get_Item(iface, &var0, &res);
				VariantClear(&var0);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x00000001: {
			if (wFlags & DISPATCH_METHOD) {
				// line 117: [id(0x00000001)]HRESULT Add([in] VARIANT* Key, [in] VARIANT* Item);
				if (pDispParams->cArgs < 2) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_Add(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 120: [id(0x00000002), propget]HRESULT Count([out, retval] long* pCount);
				V_VT(&res) = VT_I4;
				hres = dictionary_get_Count(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x00000003: {
			if (wFlags & DISPATCH_METHOD) {
				// line 123: [id(0x00000003)]HRESULT Exists([in] VARIANT* Key, [out, retval] VARIANT_BOOL* pExists);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = dictionary_Exists(iface, &var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000004: {
			if (wFlags & DISPATCH_METHOD) {
				// line 126: [id(0x00000004)]HRESULT Items([out, retval] VARIANT* pItemsArray);
				hres = dictionary_Items(iface, &res);
			}
			break;
		}
		case 0x00000005: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 129: [id(0x00000005), propput]HRESULT Key([in] VARIANT* Key, [in] VARIANT* rhs);
				if (pDispParams->cArgs < 2) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_put_Key(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000006: {
			if (wFlags & DISPATCH_METHOD) {
				// line 132: [id(0x00000006)]HRESULT Keys([out, retval] VARIANT* pKeysArray);
				hres = dictionary_Keys(iface, &res);
			}
			break;
		}
		case 0x00000007: {
			if (wFlags & DISPATCH_METHOD) {
				// line 135: [id(0x00000007)]HRESULT Remove([in] VARIANT* Key);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = dictionary_Remove(iface, &var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000008: {
			if (wFlags & DISPATCH_METHOD) {
				// line 138: [id(0x00000008)]HRESULT RemoveAll();
				hres = dictionary_RemoveAll(iface);
			}
			break;
		}
		case 0x00000009: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 141: [id(0x00000009), propput]HRESULT CompareMode([in] CompareMethod pcomp);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = dictionary_put_CompareMode(iface, (CompareMethod)V_I4(&var0));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYGET) {
				// line 144: [id(0x00000009), propget]HRESULT CompareMode([out, retval] CompareMethod* pcomp);
				V_VT(&res) = VT_I4;
				hres = dictionary_get_CompareMode(iface, (CompareMethod*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_METHOD) {
				// line 147: [id(DISPID_NEWENUM), restricted]HRESULT _NewEnum([out, retval] IUnknown** ppunk);
				V_VT(&res) = VT_UNKNOWN;
				hres = dictionary__NewEnum(iface, &V_UNKNOWN(&res));
			}
			break;
		}
		case 0x0000000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 150: [id(0x0000000a), propget, hidden]HRESULT HashVal([in] VARIANT* Key, [out, retval] VARIANT* HashVal);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = dictionary_get_HashVal(iface, &var0, &res);
				VariantClear(&var0);
			}
			break;
		}
		default:
		break;
	}
	if (SUCCEEDED(hres)) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else {
		libwinevbs_log(LIBWINEVBS_LOG_WARN, "dictionary_Invoke: %s (dispId=%d 0x%08x), wFlags=%d, hres=0x%08x (%s)", dictionary_dispid_name(dispIdMember), dispIdMember, dispIdMember, wFlags, hres, libwinevbs_hresult_name(hres));
	}
	return hres;
}

