#include "libwinevbs.h"

static HRESULT WINAPI WshCollection_GetIDsOfNames(IWshCollection *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Count", 1 },
			{ L"Item", DISPID_VALUE },
			{ L"length", 2 }
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

static const char *WshCollection_dispid_name(DISPID dispId)
{
	switch(dispId) {
		case DISPID_VALUE: return "Item";
		case 1: return "Count";
		case 2: return "length";
		case DISPID_NEWENUM: return "_NewEnum";
		default: return "?";
	}
}

static HRESULT WINAPI WshCollection_Invoke(IWshCollection *iface, DISPID dispIdMember,
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
			if (wFlags & DISPATCH_METHOD) {
				// line 374: [id(DISPID_VALUE)]HRESULT Item([in] VARIANT *Index,[out, retval] VARIANT *out_Value);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = WshCollection_Item(iface, &var0, &res);
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
		case 1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 379: [id(1)]HRESULT Count([out, retval] long *out_Count);
				V_VT(&res) = VT_I4;
				hres = WshCollection_Count(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 382: [id(2), propget]HRESULT length([out, retval] long *out_Count);
				V_VT(&res) = VT_I4;
				hres = WshCollection_get_length(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_METHOD) {
				// line 385: [id(DISPID_NEWENUM)]HRESULT _NewEnum([out, retval] IUnknown **out_Enum);
				V_VT(&res) = VT_UNKNOWN;
				hres = WshCollection__NewEnum(iface, &V_UNKNOWN(&res));
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
		libwinevbs_log(LIBWINEVBS_LOG_WARN, "WshCollection_Invoke: %s (dispId=%d 0x%08x), wFlags=%d, hres=0x%08x (%s)", WshCollection_dispid_name(dispIdMember), dispIdMember, dispIdMember, wFlags, hres, libwinevbs_hresult_name(hres));
	}
	return hres;
}

static HRESULT WINAPI WshEnvironment_GetIDsOfNames(IWshEnvironment *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Count", 1 },
			{ L"Item", DISPID_VALUE },
			{ L"length", 2 },
			{ L"Remove", 0x03e9 }
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

static const char *WshEnvironment_dispid_name(DISPID dispId)
{
	switch(dispId) {
		case DISPID_VALUE: return "Item";
		case 1: return "Count";
		case 2: return "length";
		case DISPID_NEWENUM: return "_NewEnum";
		case 0x03e9: return "Remove";
		default: return "?";
	}
}

static HRESULT WINAPI WshEnvironment_Invoke(IWshEnvironment *iface, DISPID dispIdMember,
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
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 396: [id(DISPID_VALUE), propget]HRESULT Item([in] BSTR Name,[out, retval] BSTR *out_Value);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = WshEnvironment_get_Item(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 401: [id(DISPID_VALUE), propput]HRESULT Item([in] BSTR Name,[in] BSTR Value);
				if (pDispParams->cArgs < 2) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshEnvironment_put_Item(iface, V_BSTR(&var0), V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 406: [id(1)]HRESULT Count([out, retval] long *out_Count);
				V_VT(&res) = VT_I4;
				hres = WshEnvironment_Count(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 409: [id(2), propget]HRESULT length([out, retval] long *out_Count);
				V_VT(&res) = VT_I4;
				hres = WshEnvironment_get_length(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_METHOD) {
				// line 412: [id(DISPID_NEWENUM)]HRESULT _NewEnum([out, retval] IUnknown **out_Enum);
				V_VT(&res) = VT_UNKNOWN;
				hres = WshEnvironment__NewEnum(iface, &V_UNKNOWN(&res));
			}
			break;
		}
		case 0x03e9: {
			if (wFlags & DISPATCH_METHOD) {
				// line 415: [id(0x03e9)]HRESULT Remove([in] BSTR Name);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshEnvironment_Remove(iface, V_BSTR(&var0));
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
		libwinevbs_log(LIBWINEVBS_LOG_WARN, "WshEnvironment_Invoke: %s (dispId=%d 0x%08x), wFlags=%d, hres=0x%08x (%s)", WshEnvironment_dispid_name(dispIdMember), dispIdMember, dispIdMember, wFlags, hres, libwinevbs_hresult_name(hres));
	}
	return hres;
}

static HRESULT WINAPI WshExec_GetIDsOfNames(IWshExec *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"ExitCode", 0x0007 },
			{ L"ProcessID", 0x0006 },
			{ L"Status", 0x0001 },
			{ L"StdErr", 0x0005 },
			{ L"StdIn", 0x0003 },
			{ L"StdOut", 0x0004 },
			{ L"Terminate", 0x0008 }
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

static const char *WshExec_dispid_name(DISPID dispId)
{
	switch(dispId) {
		case DISPID_VALUE: return "(default)";
		case 0x0001: return "Status";
		case 0x0003: return "StdIn";
		case 0x0004: return "StdOut";
		case 0x0005: return "StdErr";
		case 0x0006: return "ProcessID";
		case 0x0007: return "ExitCode";
		case 0x0008: return "Terminate";
		default: return "?";
	}
}

static HRESULT WINAPI WshExec_Invoke(IWshExec *iface, DISPID dispIdMember,
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
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x0001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 426: [id(0x0001), propget]HRESULT Status([out, retval] WshExecStatus* Status);
				V_VT(&res) = VT_I4;
				hres = WshExec_get_Status(iface, (WshExecStatus*)&V_I4(&res));
			}
			break;
		}
		case 0x0003: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 429: [id(0x0003), propget]HRESULT StdIn([out, retval] ITextStream** stream);
				V_VT(&res) = VT_DISPATCH;
				hres = WshExec_get_StdIn(iface, (ITextStream**)&V_DISPATCH(&res));
				if (SUCCEEDED(hres) && pDispParams->cArgs > 0) {
					IDispatch *_chained = V_DISPATCH(&res);
					V_VT(&res) = VT_EMPTY;
					hres = IDispatch_Invoke(_chained, DISPID_VALUE, &IID_NULL, lcid, wFlags, pDispParams, &res, pExcepInfo, puArgErr);
					IDispatch_Release(_chained);
				}
			}
			break;
		}
		case 0x0004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 432: [id(0x0004), propget]HRESULT StdOut([out, retval] ITextStream** stream);
				V_VT(&res) = VT_DISPATCH;
				hres = WshExec_get_StdOut(iface, (ITextStream**)&V_DISPATCH(&res));
				if (SUCCEEDED(hres) && pDispParams->cArgs > 0) {
					IDispatch *_chained = V_DISPATCH(&res);
					V_VT(&res) = VT_EMPTY;
					hres = IDispatch_Invoke(_chained, DISPID_VALUE, &IID_NULL, lcid, wFlags, pDispParams, &res, pExcepInfo, puArgErr);
					IDispatch_Release(_chained);
				}
			}
			break;
		}
		case 0x0005: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 435: [id(0x0005), propget]HRESULT StdErr([out, retval] ITextStream** stream);
				V_VT(&res) = VT_DISPATCH;
				hres = WshExec_get_StdErr(iface, (ITextStream**)&V_DISPATCH(&res));
				if (SUCCEEDED(hres) && pDispParams->cArgs > 0) {
					IDispatch *_chained = V_DISPATCH(&res);
					V_VT(&res) = VT_EMPTY;
					hres = IDispatch_Invoke(_chained, DISPID_VALUE, &IID_NULL, lcid, wFlags, pDispParams, &res, pExcepInfo, puArgErr);
					IDispatch_Release(_chained);
				}
			}
			break;
		}
		case 0x0006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 438: [id(0x0006), propget]HRESULT ProcessID([out, retval] int *pid);
				V_VT(&res) = VT_I4;
				hres = WshExec_get_ProcessID(iface, (int*)&V_I4(&res));
			}
			break;
		}
		case 0x0007: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 441: [id(0x0007), propget]HRESULT ExitCode([out, retval] int *code);
				V_VT(&res) = VT_I4;
				hres = WshExec_get_ExitCode(iface, (int*)&V_I4(&res));
			}
			break;
		}
		case 0x0008: {
			if (wFlags & DISPATCH_METHOD) {
				// line 444: [id(0x0008)]HRESULT Terminate();
				hres = WshExec_Terminate(iface);
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
		libwinevbs_log(LIBWINEVBS_LOG_WARN, "WshExec_Invoke: %s (dispId=%d 0x%08x), wFlags=%d, hres=0x%08x (%s)", WshExec_dispid_name(dispIdMember), dispIdMember, dispIdMember, wFlags, hres, libwinevbs_hresult_name(hres));
	}
	return hres;
}

static HRESULT WINAPI WshShortcut_GetIDsOfNames(IWshShortcut *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Arguments", 0x03e8 },
			{ L"Description", 0x03e9 },
			{ L"FullName", DISPID_VALUE },
			{ L"Hotkey", 0x03ea },
			{ L"IconLocation", 0x03eb },
			{ L"Load", 0x07d0 },
			{ L"RelativePath", 0x03ec },
			{ L"Save", 0x07d1 },
			{ L"TargetPath", 0x03ed },
			{ L"WindowStyle", 0x03ee },
			{ L"WorkingDirectory", 0x03ef }
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

static const char *WshShortcut_dispid_name(DISPID dispId)
{
	switch(dispId) {
		case DISPID_VALUE: return "FullName";
		case 0x03e8: return "Arguments";
		case 0x03e9: return "Description";
		case 0x03ea: return "Hotkey";
		case 0x03eb: return "IconLocation";
		case 0x03ec: return "RelativePath";
		case 0x03ed: return "TargetPath";
		case 0x03ee: return "WindowStyle";
		case 0x03ef: return "WorkingDirectory";
		case 0x07d0: return "Load";
		case 0x07d1: return "Save";
		default: return "?";
	}
}

static HRESULT WINAPI WshShortcut_Invoke(IWshShortcut *iface, DISPID dispIdMember,
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
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 455: [id(DISPID_VALUE), propget]HRESULT FullName([out, retval] BSTR* name);
				V_VT(&res) = VT_BSTR;
				hres = WshShortcut_get_FullName(iface, &V_BSTR(&res));
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x03e8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 458: [id(0x03e8), propget]HRESULT Arguments([out, retval] BSTR* Arguments);
				V_VT(&res) = VT_BSTR;
				hres = WshShortcut_get_Arguments(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 461: [id(0x03e8), propput]HRESULT Arguments([in] BSTR Arguments);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_put_Arguments(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03e9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 464: [id(0x03e9), propget]HRESULT Description([out, retval] BSTR* Description);
				V_VT(&res) = VT_BSTR;
				hres = WshShortcut_get_Description(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 467: [id(0x03e9), propput]HRESULT Description([in] BSTR Description);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_put_Description(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03ea: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 470: [id(0x03ea), propget]HRESULT Hotkey([out, retval] BSTR* HotKey);
				V_VT(&res) = VT_BSTR;
				hres = WshShortcut_get_Hotkey(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 473: [id(0x03ea), propput]HRESULT Hotkey([in] BSTR HotKey);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_put_Hotkey(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03eb: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 476: [id(0x03eb), propget]HRESULT IconLocation([out, retval] BSTR* IconPath);
				V_VT(&res) = VT_BSTR;
				hres = WshShortcut_get_IconLocation(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 479: [id(0x03eb), propput]HRESULT IconLocation([in] BSTR IconPath);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_put_IconLocation(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03ec: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 482: [id(0x03ec), propput]HRESULT RelativePath([in] BSTR rhs);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_put_RelativePath(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03ed: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 485: [id(0x03ed), propget]HRESULT TargetPath([out, retval] BSTR* Path);
				V_VT(&res) = VT_BSTR;
				hres = WshShortcut_get_TargetPath(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 488: [id(0x03ed), propput]HRESULT TargetPath([in] BSTR Path);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_put_TargetPath(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03ee: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 491: [id(0x03ee), propget]HRESULT WindowStyle([out, retval] int* ShowCmd);
				V_VT(&res) = VT_I4;
				hres = WshShortcut_get_WindowStyle(iface, (int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 494: [id(0x03ee), propput]HRESULT WindowStyle([in] int ShowCmd);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = WshShortcut_put_WindowStyle(iface, V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03ef: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 497: [id(0x03ef), propget]HRESULT WorkingDirectory([out, retval] BSTR* WorkingDirectory);
				V_VT(&res) = VT_BSTR;
				hres = WshShortcut_get_WorkingDirectory(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 500: [id(0x03ef), propput]HRESULT WorkingDirectory([in] BSTR WorkingDirectory);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_put_WorkingDirectory(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x07d0: {
			if (wFlags & DISPATCH_METHOD) {
				// line 503: [id(0x07d0), hidden]HRESULT Load([in] BSTR PathLink);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShortcut_Load(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x07d1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 506: [id(0x07d1)]HRESULT Save();
				hres = WshShortcut_Save(iface);
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
		libwinevbs_log(LIBWINEVBS_LOG_WARN, "WshShortcut_Invoke: %s (dispId=%d 0x%08x), wFlags=%d, hres=0x%08x (%s)", WshShortcut_dispid_name(dispIdMember), dispIdMember, dispIdMember, wFlags, hres, libwinevbs_hresult_name(hres));
	}
	return hres;
}

static HRESULT WINAPI WshShell3_GetIDsOfNames(IWshShell3 *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"AppActivate", 0x0bc2 },
			{ L"CreateShortcut", 0x03ea },
			{ L"CurrentDirectory", 0x0bc5 },
			{ L"Environment", 0x00c8 },
			{ L"Exec", 0x0bc4 },
			{ L"ExpandEnvironmentStrings", 0x03ee },
			{ L"LogEvent", 0x0bb8 },
			{ L"Popup", 0x03e9 },
			{ L"RegDelete", 0x07d2 },
			{ L"RegRead", 0x07d0 },
			{ L"RegWrite", 0x07d1 },
			{ L"Run", 0x03e8 },
			{ L"SendKeys", 0x0bc3 },
			{ L"SpecialFolders", 0x0064 }
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

static const char *WshShell3_dispid_name(DISPID dispId)
{
	switch(dispId) {
		case DISPID_VALUE: return "(default)";
		case 0x0064: return "SpecialFolders";
		case 0x00c8: return "Environment";
		case 0x03e8: return "Run";
		case 0x03e9: return "Popup";
		case 0x03ea: return "CreateShortcut";
		case 0x03ee: return "ExpandEnvironmentStrings";
		case 0x07d0: return "RegRead";
		case 0x07d1: return "RegWrite";
		case 0x07d2: return "RegDelete";
		case 0x0bb8: return "LogEvent";
		case 0x0bc2: return "AppActivate";
		case 0x0bc3: return "SendKeys";
		case 0x0bc4: return "Exec";
		case 0x0bc5: return "CurrentDirectory";
		default: return "?";
	}
}

static HRESULT WINAPI WshShell3_Invoke(IWshShell3 *iface, DISPID dispIdMember,
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
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x0064: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 518: [id(0x0064), propget]HRESULT SpecialFolders([out, retval] IWshCollection** out_Folders);
				V_VT(&res) = VT_DISPATCH;
				hres = WshShell3_get_SpecialFolders(iface, (IWshCollection**)&V_DISPATCH(&res));
				if (SUCCEEDED(hres) && pDispParams->cArgs > 0) {
					IDispatch *_chained = V_DISPATCH(&res);
					V_VT(&res) = VT_EMPTY;
					hres = IDispatch_Invoke(_chained, DISPID_VALUE, &IID_NULL, lcid, wFlags, pDispParams, &res, pExcepInfo, puArgErr);
					IDispatch_Release(_chained);
				}
			}
			break;
		}
		case 0x00c8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 521: [id(0x00c8), propget]HRESULT Environment([in, optional] VARIANT* Type,[out, retval] IWshEnvironment** out_Env);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_DISPATCH;
				hres = WshShell3_get_Environment(iface, &var0, (IWshEnvironment**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03e8: {
			if (wFlags & DISPATCH_METHOD) {
				// line 526: [id(0x03e8)]HRESULT Run([in] BSTR Command,[in, optional] VARIANT* WindowStyle,[in, optional] VARIANT* WaitOnReturn,[out, retval] int* out_ExitCode);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantCopyInd(&var2, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = WshShell3_Run(iface, V_BSTR(&var0), &var1, &var2, (int*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x03e9: {
			if (wFlags & DISPATCH_METHOD) {
				// line 533: [id(0x03e9)]HRESULT Popup([in] BSTR Text,[in, optional] VARIANT* SecondsToWait,[in, optional] VARIANT* Title,[in, optional] VARIANT* Type,[out, retval] int* out_Button);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantCopyInd(&var2, &pDispParams->rgvarg[--index]);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantCopyInd(&var3, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = WshShell3_Popup(iface, V_BSTR(&var0), &var1, &var2, &var3, (int*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x03ea: {
			if (wFlags & DISPATCH_METHOD) {
				// line 541: [id(0x03ea)]HRESULT CreateShortcut([in] BSTR PathLink,[out, retval] IDispatch** out_Shortcut);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = WshShell3_CreateShortcut(iface, V_BSTR(&var0), (IDispatch**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x03ee: {
			if (wFlags & DISPATCH_METHOD) {
				// line 546: [id(0x03ee)]HRESULT ExpandEnvironmentStrings([in] BSTR Src,[out, retval] BSTR* out_Dst);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = WshShell3_ExpandEnvironmentStrings(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x07d0: {
			if (wFlags & DISPATCH_METHOD) {
				// line 551: [id(0x07d0)]HRESULT RegRead([in] BSTR Name,[out, retval] VARIANT* out_Value);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShell3_RegRead(iface, V_BSTR(&var0), &res);
				VariantClear(&var0);
			}
			break;
		}
		case 0x07d1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 556: [id(0x07d1)]HRESULT RegWrite([in] BSTR Name,[in] VARIANT* Value,[in, optional] VARIANT* Type);
				if (pDispParams->cArgs < 2) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantCopyInd(&var2, &pDispParams->rgvarg[--index]);
				hres = WshShell3_RegWrite(iface, V_BSTR(&var0), &var1, &var2);
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x07d2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 562: [id(0x07d2)]HRESULT RegDelete([in] BSTR Name);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShell3_RegDelete(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0bb8: {
			if (wFlags & DISPATCH_METHOD) {
				// line 574: [id(0x0bb8)]HRESULT LogEvent([in] VARIANT* Type,[in] BSTR Message,[in, defaultvalue("")] BSTR Target,[out, retval] VARIANT_BOOL* out_Success);
				if (pDispParams->cArgs < 2) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_BSTR;
				V_BSTR(&var2) = SysAllocString(L"");
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = WshShell3_LogEvent(iface, &var0, V_BSTR(&var1), V_BSTR(&var2), &V_BOOL(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x0bc2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 581: [id(0x0bc2)]HRESULT AppActivate([in] VARIANT* App,[in, optional] VARIANT* Wait,[out, retval] VARIANT_BOOL* out_Success);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = WshShell3_AppActivate(iface, &var0, &var1, &V_BOOL(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0bc3: {
			if (wFlags & DISPATCH_METHOD) {
				// line 587: [id(0x0bc3)]HRESULT SendKeys([in] BSTR Keys,[in, optional] VARIANT* Wait);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = WshShell3_SendKeys(iface, V_BSTR(&var0), &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0bc4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 600: [id(0x0bc4)]HRESULT Exec([in] BSTR Command,[out, retval] IWshExec** out_Exec);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = WshShell3_Exec(iface, V_BSTR(&var0), (IWshExec**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0bc5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 605: [id(0x0bc5), propget]HRESULT CurrentDirectory([out, retval] BSTR* out_Directory);
				V_VT(&res) = VT_BSTR;
				hres = WshShell3_get_CurrentDirectory(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 608: [id(0x0bc5), propput]HRESULT CurrentDirectory([in] BSTR out_Directory);
				if (pDispParams->cArgs < 1) {
					hres = DISP_E_BADPARAMCOUNT;
					break;
				}
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = WshShell3_put_CurrentDirectory(iface, V_BSTR(&var0));
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
		libwinevbs_log(LIBWINEVBS_LOG_WARN, "WshShell3_Invoke: %s (dispId=%d 0x%08x), wFlags=%d, hres=0x%08x (%s)", WshShell3_dispid_name(dispIdMember), dispIdMember, dispIdMember, wFlags, hres, libwinevbs_hresult_name(hres));
	}
	return hres;
}

