/*
 * Copyright 2011 Jacek Caban for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "initguid.h"

#include "vbscript.h"
#include "objsafe.h"
#include "mshtmhst.h"
#include "rpcproxy.h"
#include "vbscript_classes.h"
#include "vbsglobal.h"
#include "vbsregexp55.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(vbscript);
WINE_DECLARE_DEBUG_CHANNEL(heap);

DEFINE_GUID(GUID_NULL,0,0,0,0,0,0,0,0,0,0,0);

static HINSTANCE vbscript_hinstance;
static ITypeInfo *dispatch_typeinfo;

BSTR get_vbscript_string(int id)
{
    WCHAR buf[512];
#ifndef __LIBWINEVBS__
    if(!LoadStringW(vbscript_hinstance, id, buf, ARRAY_SIZE(buf))) return NULL;
#else
    /* English VBScript error strings. Generated from the vendored Wine resource
       file wine/dlls/vbscript/vbscript.rc by scripts/genvbserrors.sh; libwinevbs
       does not link the compiled resources, so we look them up here instead of
       via LoadStringW. Re-run that script after bumping Wine to refresh them. */
    static const struct {
        int id;
        const WCHAR *str;
    } error_strings[] = {
        /* BEGIN GENERATED ERROR STRINGS - do not edit by hand, see scripts/genvbserrors.sh */
        { VBSE_ILLEGAL_FUNC_CALL,             L"Invalid procedure call or argument" },
        { VBSE_OVERFLOW,                      L"Overflow" },
        { VBSE_OUT_OF_MEMORY,                 L"Out of memory" },
        { VBSE_OUT_OF_BOUNDS,                 L"Subscript out of range" },
        { VBSE_ARRAY_LOCKED,                  L"This array is fixed or temporarily locked" },
        { VBSE_DIVISION_BY_ZERO,              L"Division by zero" },
        { VBSE_TYPE_MISMATCH,                 L"Type mismatch" },
        { VBSE_OUT_OF_STACK,                  L"Out of stack space" },
        { VBSE_FILE_NOT_FOUND,                L"File not found" },
        { VBSE_IO_ERROR,                      L"Device I/O error" },
        { VBSE_FILE_ALREADY_EXISTS,           L"File already exists" },
        { VBSE_DISK_FULL,                     L"Disk full" },
        { VBSE_TOO_MANY_FILES,                L"Too many files" },
        { VBSE_PERMISSION_DENIED,             L"Permission denied" },
        { VBSE_PATH_FILE_ACCESS,              L"Path/File access error" },
        { VBSE_PATH_NOT_FOUND,                L"Path not found" },
        { VBSE_OBJECT_VARIABLE_NOT_SET,       L"Object variable not set" },
        { VBSE_FOR_LOOP_NOT_INITIALIZED,      L"For loop not initialized" },
        { VBSE_ILLEGAL_NULL_USE,              L"Invalid use of Null" },
        { VBSE_CANT_CREATE_TMP_FILE,          L"Can't create necessary temporary file" },
        { VBSE_OBJECT_REQUIRED,               L"Object required" },
        { VBSE_CANT_CREATE_OBJECT,            L"ActiveX component can't create object" },
        { VBSE_OLE_NOT_SUPPORTED,             L"Class doesn't support Automation" },
        { VBSE_OLE_FILE_NOT_FOUND,            L"File name or class name not found during Automation operation" },
        { VBSE_OLE_NO_PROP_OR_METHOD,         L"Object doesn't support this property or method" },
        { VBSE_ACTION_NOT_SUPPORTED,          L"Object doesn't support this action" },
        { VBSE_NAMED_ARGS_NOT_SUPPORTED,      L"Object doesn't support named arguments" },
        { VBSE_LOCALE_SETTING_NOT_SUPPORTED,  L"Object doesn't support current locale setting" },
        { VBSE_NAMED_PARAM_NOT_FOUND,         L"Named argument not found" },
        { VBSE_PARAMETER_NOT_OPTIONAL,        L"Argument not optional" },
        { VBSE_FUNC_ARITY_MISMATCH,           L"Wrong number of arguments or invalid property assignment" },
        { VBSE_NOT_ENUM,                      L"Object not a collection" },
        { VBSE_DUPLICATE_KEY,                 L"This key is already associated with an element of this collection" },
        { VBSE_INVALID_DLL_FUNCTION_NAME,     L"Specified DLL function not found" },
        { VBSE_INVALID_TYPELIB_VARIABLE,      L"Variable uses an Automation type not supported in VBScript" },
        { VBSE_SERVER_NOT_FOUND,              L"The remote server machine does not exist or is unavailable" },
        { VBSE_VARIABLE_UNDEFINED,            L"Variable is undefined" },
        { VBSE_ILLEGAL_ASSIGNMENT,            L"Illegal assignment" },
        { VBSE_UNQUALIFIED_REFERENCE,         L"Invalid or unqualified reference" },
        { VBSE_CLASS_NOT_DEFINED,             L"Class not defined" },
        { VBSE_SYNTAX_ERROR,                  L"Syntax error" },
        { VBSE_EXPECTED_LPAREN,               L"Expected '('" },
        { VBSE_EXPECTED_RBRACKET,             L"Expected ']'" },
        { VBSE_EXPECTED_RPAREN,               L"Expected ')'" },
        { VBSE_EXPECTED_IDENTIFIER,           L"Expected identifier" },
        { VBSE_EXPECTED_ASSIGN,               L"Expected '='" },
        { VBSE_EXPECTED_IF,                   L"Expected 'If'" },
        { VBSE_EXPECTED_TO,                   L"Expected 'To'" },
        { VBSE_EXPECTED_END,                  L"Expected 'End'" },
        { VBSE_EXPECTED_FUNCTION,             L"Expected 'Function'" },
        { VBSE_EXPECTED_SUB,                  L"Expected 'Sub'" },
        { VBSE_EXPECTED_THEN,                 L"Expected 'Then'" },
        { VBSE_EXPECTED_WEND,                 L"Expected 'Wend'" },
        { VBSE_EXPECTED_LOOP,                 L"Expected 'Loop'" },
        { VBSE_EXPECTED_NEXT,                 L"Expected 'Next'" },
        { VBSE_EXPECTED_CASE,                 L"Expected 'Case'" },
        { VBSE_EXPECTED_SELECT,               L"Expected 'Select'" },
        { VBSE_EXPECTED_STATEMENT,            L"Expected statement" },
        { VBSE_EXPECTED_END_OF_STATEMENT,     L"Expected end of statement" },
        { VBSE_EXPECTED_INTEGER_CONSTANT,     L"Expected integer constant" },
        { VBSE_EXPECTED_WHILE_UNTIL_EOS,      L"Expected 'While', 'Until' or end of statement" },
        { VBSE_IDENTIFIER_TOO_LONG,           L"Identifier too long" },
        { VBSE_EXPECTED_WITH,                 L"Expected 'With'" },
        { VBSE_INVALID_NUMBER,                L"Invalid number" },
        { VBSE_INVALID_CHAR,                  L"Invalid character" },
        { VBSE_UNTERMINATED_STRING,           L"Unterminated string constant" },
        { VBSE_INVALID_USE_OF_ME,             L"Invalid use of 'Me' keyword" },
        { VBSE_LOOP_WITHOUT_DO,               L"'loop' without 'do'" },
        { VBSE_INVALID_EXIT,                  L"Invalid 'exit' statement" },
        { VBSE_INVALID_FOR_CONTROL,           L"Invalid 'for' loop control variable" },
        { VBSE_NAME_REDEFINED,                L"Name redefined" },
        { VBSE_MUST_BE_FIRST_STATEMENT,       L"Must be first statement on the line" },
        { VBSE_CANNOT_USE_PARENS_CALLING_SUB, L"Cannot use parentheses when calling a Sub" },
        { VBSE_EXPECTED_LITERAL_CONSTANT,     L"Expected literal constant" },
        { VBSE_EXPECTED_IN,                   L"Expected 'In'" },
        { VBSE_EXPECTED_CLASS,                L"Expected 'Class'" },
        { VBSE_MUST_BE_INSIDE_CLASS,          L"Must be defined inside a Class" },
        { VBSE_EXPECTED_LET_SET_GET,          L"Expected Let or Set or Get in property declaration" },
        { VBSE_EXPECTED_PROPERTY,             L"Expected 'Property'" },
        { VBSE_PROPERTY_ARG_COUNT_MISMATCH,   L"Number of arguments must be consistent across properties specification" },
        { VBSE_MULTIPLE_DEFAULT_MEMBERS,      L"Cannot have multiple default property/method in a Class" },
        { VBSE_CLASS_INIT_NO_ARGS,            L"Class initialize or terminate do not have arguments" },
        { VBSE_PROPERTY_LET_SET_NEEDS_ARG,    L"Property set or let must have at least one argument" },
        { VBSE_UNEXPECTED_NEXT,               L"Unexpected 'Next'" },
        { VBSE_DEFAULT_MUST_BE_PUBLIC,        L"'Default' specification must also specify 'Public'" },
        { VBSE_DEFAULT_ONLY_ON_PROPERTY_GET,  L"'Default' specification can only be on Property Get" },
        { VBS_COMPILE_ERROR,                  L"Microsoft VBScript compilation error" },
        { VBS_RUNTIME_ERROR,                  L"Microsoft VBScript runtime error" },
        { VBS_UNKNOWN_RUNTIME_ERROR,          L"Unknown runtime error" },
        /* END GENERATED ERROR STRINGS */
    };
    unsigned i;
    for(i = 0; i < ARRAY_SIZE(error_strings); i++) {
        if(error_strings[i].id == id)
            return SysAllocString(error_strings[i].str);
    }
    swprintf(buf, ARRAY_SIZE(buf), L"Unknown VBScript error %d", id);
#endif
    return SysAllocString(buf);
}

#define MIN_BLOCK_SIZE  128
#define ARENA_FREE_FILLER  0xaa

static inline DWORD block_size(DWORD block)
{
    return MIN_BLOCK_SIZE << block;
}

void heap_pool_init(heap_pool_t *heap)
{
    memset(heap, 0, sizeof(*heap));
    list_init(&heap->custom_blocks);
}

void *heap_pool_alloc(heap_pool_t *heap, size_t size)
{
    struct list *list;
    void *tmp;

    size = (size+3)&~3;

    if(!heap->block_cnt) {
        if(!heap->blocks) {
            heap->blocks = malloc(sizeof(void*));
            if(!heap->blocks)
                return NULL;
        }

        tmp = malloc(block_size(0));
        if(!tmp)
            return NULL;

        heap->blocks[0] = tmp;
        heap->block_cnt = 1;
    }

    if(heap->offset + size <= block_size(heap->last_block)) {
        tmp = ((BYTE*)heap->blocks[heap->last_block])+heap->offset;
        heap->offset += size;
        return tmp;
    }

    if(size <= block_size(heap->last_block+1)) {
        if(heap->last_block+1 == heap->block_cnt) {
            tmp = realloc(heap->blocks, (heap->block_cnt+1)*sizeof(void*));
            if(!tmp)
                return NULL;

            heap->blocks = tmp;
            heap->blocks[heap->block_cnt] = malloc(block_size(heap->block_cnt));
            if(!heap->blocks[heap->block_cnt])
                return NULL;

            heap->block_cnt++;
        }

        heap->last_block++;
        heap->offset = size;
        return heap->blocks[heap->last_block];
    }

    list = malloc(size + sizeof(struct list));
    if(!list)
        return NULL;

    list_add_head(&heap->custom_blocks, list);
    return list+1;
}

void *heap_pool_grow(heap_pool_t *heap, void *mem, DWORD size, DWORD inc)
{
    void *ret;

    if(mem == (BYTE*)heap->blocks[heap->last_block] + heap->offset-size
            && heap->offset+inc < block_size(heap->last_block)) {
        heap->offset += inc;
        return mem;
    }

    ret = heap_pool_alloc(heap, size+inc);
    if(ret) /* FIXME: avoid copying for custom blocks */
        memcpy(ret, mem, size);
    return ret;
}

void heap_pool_clear(heap_pool_t *heap)
{
    struct list *tmp;

    if(!heap)
        return;

    while((tmp = list_next(&heap->custom_blocks, &heap->custom_blocks))) {
        list_remove(tmp);
        free(tmp);
    }

    if(WARN_ON(heap)) {
        DWORD i;

        for(i=0; i < heap->block_cnt; i++)
            memset(heap->blocks[i], ARENA_FREE_FILLER, block_size(i));
    }

    heap->last_block = heap->offset = 0;
    heap->mark = FALSE;
}

void heap_pool_free(heap_pool_t *heap)
{
    DWORD i;

    heap_pool_clear(heap);

    for(i=0; i < heap->block_cnt; i++)
        free(heap->blocks[i]);
    free(heap->blocks);

    heap_pool_init(heap);
}

heap_pool_t *heap_pool_mark(heap_pool_t *heap)
{
    if(heap->mark)
        return NULL;

    heap->mark = TRUE;
    return heap;
}

HRESULT get_dispatch_typeinfo(ITypeInfo **out)
{
    ITypeInfo *typeinfo;
    ITypeLib *typelib;
    HRESULT hr;

    if (!dispatch_typeinfo)
    {
        hr = LoadRegTypeLib(&IID_StdOle, STDOLE_MAJORVERNUM, STDOLE_MINORVERNUM, STDOLE_LCID, &typelib);
        if (FAILED(hr)) return hr;

        hr = ITypeLib_GetTypeInfoOfGuid(typelib, &IID_IDispatch, &typeinfo);
        ITypeLib_Release(typelib);
        if (FAILED(hr)) return hr;

        if (InterlockedCompareExchangePointer((void**)&dispatch_typeinfo, typeinfo, NULL))
            ITypeInfo_Release(typeinfo);
    }

    *out = dispatch_typeinfo;
    return S_OK;
}

static HRESULT WINAPI ClassFactory_QueryInterface(IClassFactory *iface, REFIID riid, void **ppv)
{
    *ppv = NULL;

    if(IsEqualGUID(&IID_IUnknown, riid)) {
        TRACE("(%p)->(IID_IUnknown %p)\n", iface, ppv);
        *ppv = iface;
    }else if(IsEqualGUID(&IID_IClassFactory, riid)) {
        TRACE("(%p)->(IID_IClassFactory %p)\n", iface, ppv);
        *ppv = iface;
    }

    if(*ppv) {
        IUnknown_AddRef((IUnknown*)*ppv);
        return S_OK;
    }

    FIXME("(%p)->(%s %p)\n", iface, debugstr_guid(riid), ppv);
    return E_NOINTERFACE;
}

static ULONG WINAPI ClassFactory_AddRef(IClassFactory *iface)
{
    TRACE("(%p)\n", iface);
    return 2;
}

static ULONG WINAPI ClassFactory_Release(IClassFactory *iface)
{
    TRACE("(%p)\n", iface);
    return 1;
}

static HRESULT WINAPI ClassFactory_LockServer(IClassFactory *iface, BOOL fLock)
{
    TRACE("(%p)->(%x)\n", iface, fLock);
    return S_OK;
}

static const IClassFactoryVtbl VBScriptFactoryVtbl = {
    ClassFactory_QueryInterface,
    ClassFactory_AddRef,
    ClassFactory_Release,
    VBScriptFactory_CreateInstance,
    ClassFactory_LockServer
};

static IClassFactory VBScriptFactory = { &VBScriptFactoryVtbl };

static const IClassFactoryVtbl VBScriptRegExpFactoryVtbl = {
    ClassFactory_QueryInterface,
    ClassFactory_AddRef,
    ClassFactory_Release,
    VBScriptRegExpFactory_CreateInstance,
    ClassFactory_LockServer
};

static IClassFactory VBScriptRegExpFactory = { &VBScriptRegExpFactoryVtbl };

/******************************************************************
 *              DllMain (vbscript.@)
 */
#ifndef __LIBWINEVBS__
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpv)
{
    TRACE("(%p %ld %p)\n", hInstDLL, fdwReason, lpv);

    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstDLL);
        vbscript_hinstance = hInstDLL;
        break;
    case DLL_PROCESS_DETACH:
        if (lpv) break;
        if (dispatch_typeinfo) ITypeInfo_Release(dispatch_typeinfo);
        release_regexp_typelib();
    }

    return TRUE;
}
#endif

/***********************************************************************
 *		DllGetClassObject	(vbscript.@)
 */
#ifndef __LIBWINEVBS__
HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    if(IsEqualGUID(&CLSID_VBScript, rclsid)) {
        TRACE("(CLSID_VBScript %s %p)\n", debugstr_guid(riid), ppv);
        return IClassFactory_QueryInterface(&VBScriptFactory, riid, ppv);
    }else if(IsEqualGUID(&CLSID_VBScriptRegExp, rclsid)) {
        TRACE("(CLSID_VBScriptRegExp %s %p)\n", debugstr_guid(riid), ppv);
        return IClassFactory_QueryInterface(&VBScriptRegExpFactory, riid, ppv);
    }

    FIXME("%s %s %p\n", debugstr_guid(rclsid), debugstr_guid(riid), ppv);
    return CLASS_E_CLASSNOTAVAILABLE;
}
#endif
