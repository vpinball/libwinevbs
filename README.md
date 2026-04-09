# libwinevbs

A cross-platform VBScript engine extracted from [Wine](https://www.winehq.org/), packaged as a standalone C library.

## Building

#### Linux (x64)
```shell
sudo apt install bison
```
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DPLATFORM=linux -DARCH=x64 -B build
cmake --build build
```

#### Linux (aarch64)
```shell
sudo apt install bison
```
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DPLATFORM=linux -DARCH=aarch64 -B build
cmake --build build
```

#### macOS (arm64)
```shell
brew install bison
```
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DPLATFORM=macos -DARCH=arm64 -B build
cmake --build build
```

#### macOS (x64)
```shell
brew install bison
```
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DPLATFORM=macos -DARCH=x64 -B build
cmake --build build
```

#### iOS (arm64)
```shell
brew install bison
```
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DPLATFORM=ios -DARCH=arm64 -B build
cmake --build build
```

#### Android (arm64-v8a)
```shell
sudo apt install bison
```
```shell
cmake -DCMAKE_BUILD_TYPE=Release -DPLATFORM=android -DARCH=arm64-v8a -B build
cmake --build build
```

#### Windows MinGW / MSYS2 UCRT64 (x64)

Requires MSYS2 with UCRT64 environment. Install dependencies:

```shell
pacman -S --noconfirm bison \
  mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-tools
```

Build (entire build runs inside the MSYS2 UCRT64 shell):

```shell
MSYSTEM=UCRT64 /c/msys64/usr/bin/bash.exe -l -c "
  cd \"$(pwd)\" &&
  cmake -DCMAKE_BUILD_TYPE=Release -DPLATFORM=win-mingw -DARCH=x64 -B build &&
  cmake --build build -- -j\$(nproc)
"
```


### Install

```bash
cmake --install build --prefix /usr/local
```

## Usage

`src/winevbs_test.c` contains a complete working example. The key steps are:

### 1. Initialize the library

```c
#define COBJMACROS
#define INITGUID

#include "libwinevbs.h"
#include <oleauto.h>

DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf,
   0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);

static void log_callback(libwinevbs_log_level_t level, const char* format, va_list args)
{
   vprintf(format, args);
   printf("\n");
}

libwinevbs_callbacks_t cb = { .log = log_callback };
libwinevbs_init(&cb);
```

### 2. Create the engine

```c
IActiveScriptParse *parser = NULL;
CoCreateInstance(&CLSID_VBScript, NULL, CLSCTX_INPROC_SERVER,
   &IID_IActiveScriptParse, (void **)&parser);

IActiveScript *engine = NULL;
ASP_QueryInterface(parser, &IID_IActiveScript, (void **)&engine);
```

The `IActiveScriptParse` COBJMACROS use the 64-bit names. Define aliases for readability:

```c
#define ASP_QueryInterface  IActiveScriptParse64_QueryInterface
#define ASP_Release         IActiveScriptParse64_Release
#define ASP_InitNew         IActiveScriptParse64_InitNew
#define ASP_ParseScriptText IActiveScriptParse64_ParseScriptText
```

### 3. Implement IActiveScriptSite

The engine requires a script site that implements `IActiveScriptSite`. This is a COM interface with a C vtable. At minimum you need:

- **GetItemInfo** — returns `IUnknown`/`ITypeInfo` for named items you add to the engine
- **OnScriptError** — receives compile and runtime errors

See `src/winevbs_test.c` for a complete minimal implementation.

### 4. Wire it up

```c
IActiveScript_SetScriptSite(engine, (IActiveScriptSite *)&site);
ASP_InitNew(parser);
IActiveScript_SetScriptState(engine, SCRIPTSTATE_STARTED);
```

### 5. Expose host objects

To give VBScript access to your application, implement `IDispatch` and register as a named item:

```c
IActiveScript_AddNamedItem(engine, L"Host",
   SCRIPTITEM_ISVISIBLE | SCRIPTITEM_GLOBALMEMBERS);
```

`SCRIPTITEM_GLOBALMEMBERS` makes the object's methods callable without a prefix (e.g., `Print "hello"` instead of `Host.Print "hello"`).

Your `IDispatch` implementation maps method names to DISPIDs in `GetIDsOfNames` and handles calls in `Invoke`. See the `HostObj_*` functions in `src/winevbs_test.c`.

### 6. Execute VBScript

```c
EXCEPINFO ei = {0};

// Evaluate an expression and get the result
VARIANT result;
VariantInit(&result);
ASP_ParseScriptText(parser, L"2 + 3 * 4",
   NULL, NULL, NULL, 0, 0,
   SCRIPTTEXT_ISEXPRESSION, &result, &ei);
// result.lVal == 14

// Execute statements
ASP_ParseScriptText(parser,
   L"Dim i\n"
   L"For i = 1 To 5\n"
   L"   Print CStr(i)\n"
   L"Next\n",
   NULL, NULL, NULL, 0, 0, 0, NULL, &ei);
```

### 7. Cleanup

```c
IActiveScript_Close(engine);
IActiveScript_Release(engine);
ASP_Release(parser);
libwinevbs_shutdown();
```

### Testing

The test program is built automatically with the library:

```bash
./build/winevbs_test
```

Expected output:

```
--- Test 1: Expression ---
2 + 3 * 4 = 14

--- Test 2: Host Print ---
VBScript> Hello from VBScript!

--- Test 3: Loop + String ---
VBScript> Count: 1 squared = 1
VBScript> Count: 2 squared = 4
VBScript> Count: 3 squared = 9
VBScript> Count: 4 squared = 16
VBScript> Count: 5 squared = 25
VBScript> Upper: HELLO WORLD

Done.
```

## What's Included

- **Wine VBScript engine** — compiler, interpreter, lexer, and parser
- **OLE Automation (oleaut32)** — `VARIANT`, `SAFEARRAY`, type coercion, and `VarFormat`
- **VBScript RegExp** — `RegExp`, `Match`, and `Matches` objects
- **Scripting Runtime (scrrun)** — `Scripting.Dictionary` and `Scripting.FileSystemObject`
- **Kernelbase locale stubs** — minimal locale support for string operations

## Background

[Visual Pinball](https://github.com/vpinball/vpinball) uses VBScript as its scripting engine for table logic. On Windows, this is handled natively, but cross-platform support (macOS, Linux, iOS, Android) required an alternative. The Wine project's VBScript engine provided a solid foundation, but needed significant bug fixes and missing feature implementations to handle the breadth of real-world VBScript used in pinball tables.

`libwinevbs` packages this work as a reusable C library with a clean callback-based API, decoupled from both Wine and Visual Pinball internals.

## Wine Source Base

Wine source is available [here](https://gitlab.winehq.org/wine/wine). All local modifications are guarded with `#ifdef __LIBWINEVBS__` / `#ifndef __LIBWINEVBS__`.

To compare against upstream, Wine must be built first since IDL compilation generates the header files:

```bash
# macOS
brew install llvm lld bison

git clone https://gitlab.winehq.org/wine/wine.git /path/to/wine
cd /path/to/wine
git checkout afba987dda1430dc0d9f36fc6f27cd3584bce79e
export PATH="/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/bison/bin:$PATH"
./configure --without-freetype --enable-win64
make -j$(nproc)
```

Then diff against it:

```bash
cd /path/to/libwinevbs
./scripts/winediff.sh /path/to/wine
```

## Wine Issues

Using the Wine VBScript engine source for Visual Pinball has surfaced a large number of defects. Many of these have since been fixed in upstream Wine as a direct result of this project.

### Resolved Defects

- [Bug 49908](https://bugs.winehq.org/show_bug.cgi?id=49908) — vbscript: ExecuteGlobal is not implemented [11.6](https://gitlab.winehq.org/wine/wine/-/merge_requests/10368)
- [Bug 53644](https://bugs.winehq.org/show_bug.cgi?id=53644) — vbscript can not compile classes with lists of private / public / dim declarations [10.1](https://gitlab.winehq.org/wine/wine/-/merge_requests/7068)
- [Bug 53670](https://bugs.winehq.org/show_bug.cgi?id=53670) — vbscript can not compile if expressions with reversed gte, lte, (=>, =<) [7.22](https://gitlab.winehq.org/wine/wine/-/merge_requests/1534)
- [Bug 53676](https://bugs.winehq.org/show_bug.cgi?id=53676) — vbscript can not exec_script - invalid number of arguments for Randomize [7.21](https://gitlab.winehq.org/wine/wine/-/merge_requests/1246)
- [Bug 53678](https://bugs.winehq.org/show_bug.cgi?id=53678) — vbscript can not compile CaseClausules that do not use a colon [8.0-rc1](https://gitlab.winehq.org/wine/wine/-/merge_requests/1611)
- [Bug 53766](https://bugs.winehq.org/show_bug.cgi?id=53766) — vbscript fails to handle SAFEARRAY assignment, access, UBounds, LBounds [Invalid]
- [Bug 53767](https://bugs.winehq.org/show_bug.cgi?id=53767) — vbscript fails to handle ReDim when variable is not yet created [10.16](https://gitlab.winehq.org/wine/wine/-/merge_requests/8998)
- [Bug 53782](https://bugs.winehq.org/show_bug.cgi?id=53782) — vbscript can not compile ReDim with list of variables [7.22](https://gitlab.winehq.org/wine/wine/-/merge_requests/1328)
- [Bug 53783](https://bugs.winehq.org/show_bug.cgi?id=53783) — vbscript can not compile private const expressions [8.0-rc1](https://gitlab.winehq.org/wine/wine/-/merge_requests/1561)
- [Bug 53807](https://bugs.winehq.org/show_bug.cgi?id=53807) — vbscript fails to redim original array in function when passed byref [7.22](https://gitlab.winehq.org/wine/wine/-/merge_requests/1495)
- [Bug 53844](https://bugs.winehq.org/show_bug.cgi?id=53844) — invoke_vbdisp not handling let property correctly for VT_DISPATCH arguments [11.6](https://gitlab.winehq.org/wine/wine/-/merge_requests/10373)
- [Bug 53866](https://bugs.winehq.org/show_bug.cgi?id=53866) — vbscript fails to handle SAFEARRAY in for...each [Invalid]
- [Bug 53867](https://bugs.winehq.org/show_bug.cgi?id=53867) — vbscript fails to retrieve property array by index [7.22](https://gitlab.winehq.org/wine/wine/-/merge_requests/1409)
- [Bug 53868](https://bugs.winehq.org/show_bug.cgi?id=53868) — vbscript fails to return TypeName for VT_DISPATCH [7.22](https://gitlab.winehq.org/wine/wine/-/merge_requests/1347)
- [Bug 53873](https://bugs.winehq.org/show_bug.cgi?id=53873) — vbscript fails to compile Else If when If is on same line [7.22](https://gitlab.winehq.org/wine/wine/-/merge_requests/1385)
- [Bug 53888](https://bugs.winehq.org/show_bug.cgi?id=53888) — vbscript does not allow Mid on non VT_BSTR [7.21](https://gitlab.winehq.org/wine/wine/-/merge_requests/1288)
- [Bug 53889](https://bugs.winehq.org/show_bug.cgi?id=53889) — does not support Get_Item call on IDispatch objects [11.5](https://gitlab.winehq.org/wine/wine/-/merge_requests/10367)
- [Bug 53962](https://bugs.winehq.org/show_bug.cgi?id=53962) — vbscript does not Eval implemented [11.6](https://gitlab.winehq.org/wine/wine/-/merge_requests/10368)
- [Bug 54221](https://bugs.winehq.org/show_bug.cgi?id=54221) — vbscript: missing support for GetRef [11.6](https://gitlab.winehq.org/wine/wine/-/merge_requests/10444)
- [Bug 54234](https://bugs.winehq.org/show_bug.cgi?id=54234) — vbscript fails to compile when colon follows Else in If...Else [8.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/2142)
- [Bug 54291](https://bugs.winehq.org/show_bug.cgi?id=54291) — stuck in endless for loop when UBound on Empty and On Error Resume Next [11.7](https://gitlab.winehq.org/wine/wine/-/merge_requests/10366)
- [Bug 54456](https://bugs.winehq.org/show_bug.cgi?id=54456) — vbscript memory leak in For Each with SafeArray as group [8.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/2141)
- [Bug 54457](https://bugs.winehq.org/show_bug.cgi?id=54457) — vbscript memory leaks in interp_redim_preserve [8.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/2132)
- [Bug 54458](https://bugs.winehq.org/show_bug.cgi?id=54458) — vbscript memory leaks in Global_Split [8.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/2131)
- [Bug 54489](https://bugs.winehq.org/show_bug.cgi?id=54489) — vbscript Abs on BSTR returns invalid value [8.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/2175)
- [Bug 54490](https://bugs.winehq.org/show_bug.cgi?id=54490) — vbscript fails to compile when statement follows ElseIf [8.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/2188)
- [Bug 54493](https://bugs.winehq.org/show_bug.cgi?id=54493) — vbscript fails to compile concat when used without space and expression begins with H [8.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/2214)
- [Bug 54731](https://bugs.winehq.org/show_bug.cgi?id=54731) — vbscript: stack_pop_bool doesn't support floats or ole color [9.0-rc1](https://gitlab.winehq.org/wine/wine/-/merge_requests/3132)
- [Bug 54978](https://bugs.winehq.org/show_bug.cgi?id=54978) — vbscript fails to compile Sub when End Sub on same line [8.12](https://gitlab.winehq.org/wine/wine/-/merge_requests/3206)
- [Bug 55037](https://bugs.winehq.org/show_bug.cgi?id=55037) — colon on new line after Then fails [11.5](https://gitlab.winehq.org/wine/wine/-/merge_requests/10311)
- [Bug 55042](https://bugs.winehq.org/show_bug.cgi?id=55042) — IDictionary::Add() fails to add entries with numerical keys that have the same hashes [8.11](https://gitlab.winehq.org/wine/wine/-/merge_requests/3064)
- [Bug 55052](https://bugs.winehq.org/show_bug.cgi?id=55052) — For loop where right bound is string coercion issue [9.0-rc1](https://gitlab.winehq.org/wine/wine/-/merge_requests/4615)
- [Bug 55006](https://bugs.winehq.org/show_bug.cgi?id=55006) — single line if else without else body fails compilation [11.5](https://gitlab.winehq.org/wine/wine/-/merge_requests/10310)
- [Bug 55093](https://bugs.winehq.org/show_bug.cgi?id=55093) — if boolean condition should work without braces [11.6](https://gitlab.winehq.org/wine/wine/-/merge_requests/10317)
- [Bug 55185](https://bugs.winehq.org/show_bug.cgi?id=55185) — vbscript round does not handle numdecimalplaces argument [9.0-rc1](https://gitlab.winehq.org/wine/wine/-/merge_requests/4569)
- [Bug 55931](https://bugs.winehq.org/show_bug.cgi?id=55931) — vbscript: empty MOD 100000 returns garbage instead of 0 [9.0-rc1](https://gitlab.winehq.org/wine/wine/-/merge_requests/4515)
- [Bug 55969](https://bugs.winehq.org/show_bug.cgi?id=55969) — vbscript fails to return TypeName for Nothing [9.0-rc1](https://gitlab.winehq.org/wine/wine/-/merge_requests/4564)
- [Bug 56139](https://bugs.winehq.org/show_bug.cgi?id=56139) — scrrun: Dictionary does not allow storing at key Undefined [9.3](https://gitlab.winehq.org/wine/wine/-/merge_requests/4778)
- [Bug 56464](https://bugs.winehq.org/show_bug.cgi?id=56464) — vbscript: Join on array with "empty" items fails [10.2](https://gitlab.winehq.org/wine/wine/-/merge_requests/7304)
- [Bug 56480](https://bugs.winehq.org/show_bug.cgi?id=56480) — underscore line continue issues [11.5](https://gitlab.winehq.org/wine/wine/-/merge_requests/10312)
- [Bug 56781](https://bugs.winehq.org/show_bug.cgi?id=56781) — scrrun: Dictionary setting item to object fails [9.11](https://gitlab.winehq.org/wine/wine/-/merge_requests/5795)
- [Bug 56931](https://bugs.winehq.org/show_bug.cgi?id=56931) — Const used before declaration fails (explicit) [11.5](https://gitlab.winehq.org/wine/wine/-/merge_requests/10302)
- [Bug 57511](https://bugs.winehq.org/show_bug.cgi?id=57511) — For loop where loop var is not defined throws error without context [11.7](https://gitlab.winehq.org/wine/wine/-/merge_requests/10374)
- [Bug 57563](https://bugs.winehq.org/show_bug.cgi?id=57563) — vbscript: mid() throws when passed VT_EMPTY instead of returning empty string [10.1](https://gitlab.winehq.org/wine/wine/-/merge_requests/7049)
- [Bug 58248](https://bugs.winehq.org/show_bug.cgi?id=58248) — Me(Idx) fails to compile [11.5](https://gitlab.winehq.org/wine/wine/-/merge_requests/10371)
- [Bug 58026](https://bugs.winehq.org/show_bug.cgi?id=58026) — vbscript: Script running error when Dictionary contains array [11.6](https://gitlab.winehq.org/wine/wine/-/merge_requests/10308)
- [Bug 58051](https://bugs.winehq.org/show_bug.cgi?id=58051) — Dictionary direct Keys/Items access causes parse error [11.7](https://gitlab.winehq.org/wine/wine/-/merge_requests/10362)
- [Bug 58056](https://bugs.winehq.org/show_bug.cgi?id=58056) — directly indexing a Split returns Empty [11.6](https://gitlab.winehq.org/wine/wine/-/merge_requests/10308)

### Open Defects

- [Bug 53877](https://bugs.winehq.org/show_bug.cgi?id=53877) — compile_assignment assertion when assigning multidimensional array by indices [MR](https://gitlab.winehq.org/wine/wine/-/merge_requests/10363)
- [Bug 54177](https://bugs.winehq.org/show_bug.cgi?id=54177) — vbscript: Fix Sub first argument parentheses handling. [MR](https://gitlab.winehq.org/wine/wine/-/merge_requests/10244) [MR](https://gitlab.winehq.org/wine/wine/-/merge_requests/10692)
- [Bug 56280](https://bugs.winehq.org/show_bug.cgi?id=56280) — string coerced to Integer instead of Long [MR](https://gitlab.winehq.org/wine/wine/-/merge_requests/8635)
- [Bug 56281](https://bugs.winehq.org/show_bug.cgi?id=56281) — string number converted to ascii value instead of parsed value [MR](https://gitlab.winehq.org/wine/wine/-/merge_requests/10314)

### Implemented MRs (without bug reports)

- vbscript: Support element access on public array properties of class instances. [MR](https://gitlab.winehq.org/wine/wine/-/merge_requests/10383)
- vbscript: Convert VT_DISPATCH arguments to string in Eval/Execute/ExecuteGlobal. [MR](https://gitlab.winehq.org/wine/wine/-/merge_requests/10600)

### Enhancements

- vbscript: Use indexed lookup for global functions and variables. [11.7](https://gitlab.winehq.org/wine/wine/-/merge_requests/10546)

## VBScript Quirks and Workarounds

These issues stem from differences in how VBScript is interpreted in the Wine engine compared to native Windows.

### Setting values in a 2D array

This is one of the most common issues. It can be automatically patched by [vpxtool](https://github.com/francisdb/vpxtool).

```vbscript
' does not work
DTArray(i)(4) = DTCheckBrick(Activeball, DTArray(i)(2))

' workaround — use a class with named properties instead
DTArray(i).animate = DTCheckBrick(Activeball, DTArray(i).prim)
```

See: [Bug 53877](https://bugs.winehq.org/show_bug.cgi?id=53877)

### Evals fail when setting a 2D array

```vbscript
' does not work
dy = -1*(EVAL("roachxy" & xx)(1)(roachstep) - EVAL("roachxy" & xx)(1)(roachstep-1))

' workaround
dim roachxy : roachxy = EVAL("roachxy" & xx)
dy = -1*(roachxy(1)(roachstep) - roachxy(1)(roachstep-1))
```

## Acknowledgments

Wine and the people who support the VBScript engine:
- Robert Wilhelm
- Nikolay Sivov
- Jacek Caban

Special thanks to [@francisdb](https://github.com/francisdb) for providing a majority of the [vpx-standalone-scripts](https://github.com/jsm174/vpx-standalone-scripts) patches and getting many of the Wine bugs fixed in upstream Wine.


## License

This library contains code from the [Wine project](https://www.winehq.org/), licensed under the GNU Lesser General Public License v2.1 (LGPL-2.1), and the [ReactOS project](https://reactos.org/), licensed under the GNU General Public License v2 (GPL-2.0).
