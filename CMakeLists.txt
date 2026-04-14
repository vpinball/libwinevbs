cmake_minimum_required(VERSION 3.25)

set(PLATFORM "win-mingw" CACHE STRING "Platform")
set(ARCH "x64" CACHE STRING "Arch")

option(BUILD_SHARED "Option to build shared library" ON)
option(BUILD_STATIC "Option to build static library" ON)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan for Debug builds" OFF)

message(STATUS "PLATFORM: ${PLATFORM}")
message(STATUS "ARCH: ${ARCH}")

message(STATUS "BUILD_SHARED: ${BUILD_SHARED}")
message(STATUS "BUILD_STATIC: ${BUILD_STATIC}")

if(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator")
   set(CMAKE_SYSTEM_NAME iOS)
   if (PLATFORM STREQUAL "ios-simulator")
      set(CMAKE_OSX_SYSROOT iphonesimulator)
   endif()
   set(CMAKE_OSX_DEPLOYMENT_TARGET 17.0)
elseif(PLATFORM STREQUAL "tvos")
   set(CMAKE_SYSTEM_NAME tvOS)
   set(CMAKE_OSX_DEPLOYMENT_TARGET 17.0)
elseif(PLATFORM STREQUAL "android")
   set(CMAKE_SYSTEM_NAME Android)
   set(CMAKE_SYSTEM_VERSION 30)
   set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
endif()

file(READ include/libwinevbs.h version)
string(REGEX MATCH "LIBWINEVBS_VERSION_MAJOR[ ]+([0-9]+)" _tmp ${version})
set(VERSION_MAJOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "LIBWINEVBS_VERSION_MINOR[ ]+([0-9]+)" _tmp ${version})
set(VERSION_MINOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "LIBWINEVBS_VERSION_PATCH[ ]+([0-9]+)" _tmp ${version})
set(VERSION_PATCH "${CMAKE_MATCH_1}")

project(libwinevbs VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" LANGUAGES C)

if(PLATFORM STREQUAL "win-mingw")
   if(ARCH STREQUAL "x86")
      add_compile_definitions(WIN32)
   endif()
elseif(PLATFORM STREQUAL "macos")
   if (ARCH STREQUAL "arm64")
      set(CMAKE_OSX_ARCHITECTURES arm64)
   elseif(ARCH STREQUAL "x64")
      set(CMAKE_OSX_ARCHITECTURES x86_64)
   endif()
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
   set(CMAKE_INSTALL_RPATH "@executable_path")
elseif(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator" OR PLATFORM STREQUAL "tvos")
   set(CMAKE_OSX_ARCHITECTURES arm64)
elseif(PLATFORM STREQUAL "linux" OR PLATFORM STREQUAL "android")
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
   set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_VISIBILITY_PRESET hidden)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   if(ENABLE_SANITIZERS AND (PLATFORM STREQUAL "macos" OR PLATFORM STREQUAL "linux"))
      set(SANITIZER_FLAGS -fsanitize=address,undefined)
      add_compile_options(${SANITIZER_FLAGS} -fno-omit-frame-pointer -g)
      add_link_options(${SANITIZER_FLAGS})
   endif()
endif()

if(MINGW)
   find_program(GENDEF gendef)
   find_program(DLLTOOL dlltool)
endif()

find_package(BISON 3.8.2 REQUIRED)
bison_target(vbsparser
   ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript/parser.y
   ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript/parser.tab.c
   DEFINES_FILE ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript/parser.tab.h
)

set(LIBWINEVBS_SOURCES
   src/libwinevbs.c
   src/wine.c

   wine/dlls/oleaut32/oleaut.c
   wine/dlls/oleaut32/variant.c
   wine/dlls/oleaut32/varformat.c
   wine/dlls/oleaut32/vartype.c
   wine/dlls/oleaut32/safearray.c

   wine/dlls/vbscript/compile.c
   wine/dlls/vbscript/global.c
   wine/dlls/vbscript/interp.c
   wine/dlls/vbscript/lex.c
   wine/dlls/vbscript/regexp.c
   wine/dlls/vbscript/utils.c
   wine/dlls/vbscript/vbdisp.c
   wine/dlls/vbscript/vbregexp.c
   wine/dlls/vbscript/vbscript.c
   wine/dlls/vbscript/vbscript_main.c
   wine/dlls/vbscript/parser.tab.c

   wine/dlls/scrrun/dictionary.c
   wine/dlls/scrrun/filesystem.c
   wine/dlls/scrrun/scrrun.c

   wine/dlls/atl/atl.c

   wine/dlls/kernelbase/locale.c
)

set(LIBWINEVBS_INCLUDE_DIRS
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/wine/include
   ${CMAKE_SOURCE_DIR}/atl/include
   ${CMAKE_SOURCE_DIR}/atlmfc/include
)

set(LIBWINEVBS_PRIVATE_INCLUDE_DIRS
   ${CMAKE_SOURCE_DIR}/wine/dlls/vbscript
   ${CMAKE_SOURCE_DIR}/wine/dlls/oleaut32
   ${CMAKE_SOURCE_DIR}/wine/dlls/kernelbase
   ${CMAKE_SOURCE_DIR}/wine/dlls/scrrun
   ${CMAKE_SOURCE_DIR}/wine/dlls/atl
)

set(LIBWINEVBS_COMPILE_DEFINITIONS
   __LIBWINEVBS__
   LIBWINEVBS_EXPORT
   __WINESRC__
   WINE_UNICODE_NATIVE
   WINE_NO_TRACE_MSGS
   WINE_NO_DEBUG_MSGS
   "__forceinline=__attribute__((always_inline)) inline"
)

set(LIBWINEVBS_COMPILE_OPTIONS
   -Wno-format
   $<$<NOT:$<BOOL:${MINGW}>>:-fvisibility=default>
)

if(PLATFORM STREQUAL "win-mingw" AND ARCH STREQUAL "x64")
   set(LIBWINEVBS_OUTPUT_NAME "winevbs64")
else()
   set(LIBWINEVBS_OUTPUT_NAME "winevbs")
endif()

if(BUILD_SHARED)
   add_library(winevbs_shared SHARED ${LIBWINEVBS_SOURCES})

   target_include_directories(winevbs_shared PUBLIC ${LIBWINEVBS_INCLUDE_DIRS} PRIVATE ${LIBWINEVBS_PRIVATE_INCLUDE_DIRS})
   target_compile_definitions(winevbs_shared PRIVATE ${LIBWINEVBS_COMPILE_DEFINITIONS})
   target_compile_options(winevbs_shared PRIVATE ${LIBWINEVBS_COMPILE_OPTIONS})

   if(UNIX AND NOT APPLE)
      target_link_libraries(winevbs_shared PRIVATE m)
   endif()

   if(MINGW)
      target_link_options(winevbs_shared PRIVATE -Wl,--allow-multiple-definition -Wl,--export-all-symbols)
      if(DLLTOOL)
         add_custom_command(TARGET winevbs_shared POST_BUILD
            COMMAND ${DLLTOOL} -d "${CMAKE_SOURCE_DIR}/${LIBWINEVBS_OUTPUT_NAME}.def" -l "lib${LIBWINEVBS_OUTPUT_NAME}.dll.a" -D "$<TARGET_FILE_NAME:winevbs_shared>"
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:winevbs_shared>"
         )
      endif()
   endif()

   if(WIN32)
      set_target_properties(winevbs_shared PROPERTIES PREFIX "")
   endif()

   set_target_properties(winevbs_shared PROPERTIES
      OUTPUT_NAME ${LIBWINEVBS_OUTPUT_NAME}
      VERSION ${PROJECT_VERSION}
      SOVERSION 0
   )

   install(TARGETS winevbs_shared
      LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
   )
   install(FILES ${CMAKE_SOURCE_DIR}/include/libwinevbs.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/atl/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs/atl)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/atlmfc/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs/atlmfc)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/wine/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs/wine)

   if(PLATFORM STREQUAL "win-mingw" OR PLATFORM STREQUAL "macos" OR PLATFORM STREQUAL "linux")
      add_executable(winevbs_test src/winevbs_test.c)
      target_link_libraries(winevbs_test PRIVATE winevbs_shared)
      target_compile_definitions(winevbs_test PRIVATE
         __LIBWINEVBS__
         __WINESRC__
         WINE_UNICODE_NATIVE
      )
   endif()
endif()

if(BUILD_STATIC)
   add_library(winevbs_static STATIC ${LIBWINEVBS_SOURCES})

   target_include_directories(winevbs_static PUBLIC ${LIBWINEVBS_INCLUDE_DIRS} PRIVATE ${LIBWINEVBS_PRIVATE_INCLUDE_DIRS})
   target_compile_definitions(winevbs_static PRIVATE ${LIBWINEVBS_COMPILE_DEFINITIONS})
   target_compile_options(winevbs_static PRIVATE ${LIBWINEVBS_COMPILE_OPTIONS})

   set_target_properties(winevbs_static PROPERTIES
      OUTPUT_NAME ${LIBWINEVBS_OUTPUT_NAME}
   )

   install(TARGETS winevbs_static
      LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
   )
   install(FILES ${CMAKE_SOURCE_DIR}/include/libwinevbs.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/atl/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs/atl)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/atlmfc/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs/atlmfc)
   install(DIRECTORY ${CMAKE_SOURCE_DIR}/wine/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libwinevbs/wine)
endif()
