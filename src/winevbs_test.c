#define COBJMACROS
#define INITGUID

#include "libwinevbs.h"
#include <oleauto.h>
#include <stdio.h>
#include <string.h>

DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);

#define ASP_QueryInterface IActiveScriptParse64_QueryInterface
#define ASP_Release IActiveScriptParse64_Release
#define ASP_InitNew IActiveScriptParse64_InitNew
#define ASP_ParseScriptText IActiveScriptParse64_ParseScriptText

static void log_callback(libwinevbs_log_level_t level, const char* format, va_list args)
{
   const char *prefix = level == LIBWINEVBS_LOG_ERROR ? "ERROR" :
                        level == LIBWINEVBS_LOG_WARN  ? "WARN"  :
                        level == LIBWINEVBS_LOG_DEBUG ? "DEBUG" : "INFO";
   printf("[%s] ", prefix);
   vprintf(format, args);
   printf("\n");
}

#define DISPID_PRINT 1

static HRESULT WINAPI HostObj_QueryInterface(IDispatch *iface, REFIID riid, void **ppv)
{
   if (IsEqualGUID(riid, &IID_IUnknown) || IsEqualGUID(riid, &IID_IDispatch)) {
      *ppv = iface;
      return S_OK;
   }
   *ppv = NULL;
   return E_NOINTERFACE;
}

static ULONG WINAPI HostObj_AddRef(IDispatch *iface) { return 2; }
static ULONG WINAPI HostObj_Release(IDispatch *iface) { return 1; }
static HRESULT WINAPI HostObj_GetTypeInfoCount(IDispatch *iface, UINT *pctinfo) { *pctinfo = 0; return S_OK; }
static HRESULT WINAPI HostObj_GetTypeInfo(IDispatch *iface, UINT i, LCID lcid, ITypeInfo **ppti) { return E_NOTIMPL; }

static HRESULT WINAPI HostObj_GetIDsOfNames(IDispatch *iface, REFIID riid, LPOLESTR *names, UINT count, LCID lcid, DISPID *ids)
{
   for (UINT i = 0; i < count; i++) {
      if (wcscmp(names[i], L"Print") == 0)
         ids[i] = DISPID_PRINT;
      else
         return DISP_E_UNKNOWNNAME;
   }
   return S_OK;
}

static HRESULT WINAPI HostObj_Invoke(IDispatch *iface, DISPID id, REFIID riid, LCID lcid,
   WORD flags, DISPPARAMS *params, VARIANT *result, EXCEPINFO *ei, UINT *err)
{
   if (id == DISPID_PRINT && params->cArgs >= 1) {
      VARIANT str;
      VariantInit(&str);
      if (SUCCEEDED(VariantChangeType(&str, &params->rgvarg[0], 0, VT_BSTR)))
         printf("VBScript> %ls\n", str.bstrVal);
      VariantClear(&str);
      return S_OK;
   }
   return DISP_E_MEMBERNOTFOUND;
}

static IDispatchVtbl g_host_vtbl = {
   HostObj_QueryInterface,
   HostObj_AddRef,
   HostObj_Release,
   HostObj_GetTypeInfoCount,
   HostObj_GetTypeInfo,
   HostObj_GetIDsOfNames,
   HostObj_Invoke
};

static IDispatch g_host_obj = { &g_host_vtbl };

typedef struct {
   IActiveScriptSiteVtbl *lpVtbl;
} ScriptSite;

static HRESULT WINAPI Site_QueryInterface(IActiveScriptSite *iface, REFIID riid, void **ppv)
{
   if (IsEqualGUID(riid, &IID_IUnknown) || IsEqualGUID(riid, &IID_IActiveScriptSite)) {
      *ppv = iface;
      return S_OK;
   }
   *ppv = NULL;
   return E_NOINTERFACE;
}

static ULONG WINAPI Site_AddRef(IActiveScriptSite *iface) { return 2; }
static ULONG WINAPI Site_Release(IActiveScriptSite *iface) { return 1; }
static HRESULT WINAPI Site_GetLCID(IActiveScriptSite *iface, LCID *plcid) { *plcid = 0; return S_OK; }

static HRESULT WINAPI Site_GetItemInfo(IActiveScriptSite *iface, LPCOLESTR name, DWORD mask, IUnknown **ppunk, ITypeInfo **ppti)
{
   if (ppti) *ppti = NULL;
   if (ppunk && wcscmp(name, L"Host") == 0) {
      *ppunk = (IUnknown *)&g_host_obj;
      return S_OK;
   }
   return TYPE_E_ELEMENTNOTFOUND;
}

static HRESULT WINAPI Site_GetDocVersionString(IActiveScriptSite *iface, BSTR *pbstr) { *pbstr = SysAllocString(L"1.0"); return S_OK; }
static HRESULT WINAPI Site_OnScriptTerminate(IActiveScriptSite *iface, const VARIANT *pvr, const EXCEPINFO *pei) { return S_OK; }
static HRESULT WINAPI Site_OnStateChange(IActiveScriptSite *iface, SCRIPTSTATE state) { return S_OK; }

static HRESULT WINAPI Site_OnScriptError(IActiveScriptSite *iface, IActiveScriptError *err)
{
   EXCEPINFO ei = {0};
   DWORD ctx;
   ULONG line;
   LONG col;
   IActiveScriptError_GetExceptionInfo(err, &ei);
   IActiveScriptError_GetSourcePosition(err, &ctx, &line, &col);
   printf("SCRIPT ERROR line %lu col %ld: %ls\n", (unsigned long)line, (long)col, ei.bstrDescription ? ei.bstrDescription : L"(unknown)");
   SysFreeString(ei.bstrDescription);
   SysFreeString(ei.bstrSource);
   return S_OK;
}

static HRESULT WINAPI Site_OnEnterScript(IActiveScriptSite *iface) { return S_OK; }
static HRESULT WINAPI Site_OnLeaveScript(IActiveScriptSite *iface) { return S_OK; }

static IActiveScriptSiteVtbl g_site_vtbl = {
   Site_QueryInterface,
   Site_AddRef,
   Site_Release,
   Site_GetLCID,
   Site_GetItemInfo,
   Site_GetDocVersionString,
   Site_OnScriptTerminate,
   Site_OnStateChange,
   Site_OnScriptError,
   Site_OnEnterScript,
   Site_OnLeaveScript
};

static ScriptSite g_site = { &g_site_vtbl };

int main(void)
{
   libwinevbs_callbacks_t cb = { .log = log_callback };
   libwinevbs_init(&cb);

   IActiveScriptParse *parser = NULL;
   HRESULT hr = CoCreateInstance(&CLSID_VBScript, NULL, CLSCTX_INPROC_SERVER, &IID_IActiveScriptParse, (void **)&parser);
   if (FAILED(hr)) {
      printf("Failed to create VBScript engine: 0x%08X\n", hr);
      return 1;
   }

   IActiveScript *engine = NULL;
   ASP_QueryInterface(parser, &IID_IActiveScript, (void **)&engine);

   IActiveScript_SetScriptSite(engine, (IActiveScriptSite *)&g_site);
   ASP_InitNew(parser);

   IActiveScript_AddNamedItem(engine, L"Host", SCRIPTITEM_ISVISIBLE | SCRIPTITEM_GLOBALMEMBERS);

   IActiveScript_SetScriptState(engine, SCRIPTSTATE_STARTED);

   printf("--- Test 1: Expression ---\n");
   VARIANT result;
   VariantInit(&result);
   EXCEPINFO ei = {0};
   hr = ASP_ParseScriptText(parser,
      L"2 + 3 * 4",
      NULL, NULL, NULL, 0, 0,
      SCRIPTTEXT_ISEXPRESSION, &result, &ei);
   if (SUCCEEDED(hr))
      printf("2 + 3 * 4 = %d\n", V_I4(&result));
   else
      printf("Expression failed: 0x%08X\n", hr);
   VariantClear(&result);

   printf("\n--- Test 2: Host Print ---\n");
   hr = ASP_ParseScriptText(parser,
      L"Print \"Hello from VBScript!\"",
      NULL, NULL, NULL, 0, 0,
      0, NULL, &ei);
   if (FAILED(hr))
      printf("Print test failed: 0x%08X\n", hr);

   printf("\n--- Test 3: Loop + String ---\n");
   hr = ASP_ParseScriptText(parser,
      L"Dim i\n"
      L"For i = 1 To 5\n"
      L"   Print \"Count: \" & CStr(i) & \" squared = \" & CStr(i * i)\n"
      L"Next\n"
      L"Print \"Upper: \" & UCase(\"hello world\")\n",
      NULL, NULL, NULL, 0, 0,
      0, NULL, &ei);
   if (FAILED(hr))
      printf("Loop test failed: 0x%08X\n", hr);

   printf("\n--- Test 4: WScript.Shell Reg* no-op ---\n");
   hr = ASP_ParseScriptText(parser,
      L"Dim sh\n"
      L"Set sh = CreateObject(\"WScript.Shell\")\n"
      L"sh.RegWrite \"HKCU\\Software\\Test\\Hi\", \"value\", \"REG_SZ\"\n"
      L"Print \"RegWrite ok\"\n"
      L"On Error Resume Next\n"
      L"Dim v\n"
      L"v = sh.RegRead(\"HKCU\\Software\\Test\\Missing\")\n"
      L"If Err.Number <> 0 Then\n"
      L"   Print \"RegRead failed as expected: \" & Err.Description\n"
      L"Else\n"
      L"   Print \"RegRead unexpectedly succeeded\"\n"
      L"End If\n",
      NULL, NULL, NULL, 0, 0,
      0, NULL, &ei);
   if (FAILED(hr))
      printf("WScript.Shell test failed: 0x%08X\n", hr);

   IActiveScript_Close(engine);
   IActiveScript_Release(engine);
   ASP_Release(parser);
   libwinevbs_shutdown();

   printf("\nDone.\n");
   return 0;
}
