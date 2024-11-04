cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)


set(TARGET FA18E_SH_patch)
project(${TARGET} VERSION 1.0 LANGUAGES CXX ASM_MASM)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message(FATAL_ERROR "Only MSVC compiler is supported")
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(TARGET_LOADER F18loader)
set(TARGET_PATCH F18patch)

set(INJECTION_TARGET_FILENAME "F18.EXE" CACHE STRING "Injection target filename")
set(INJECTED_PATCH_FILENAME "F18patch.dll" CACHE STRING "Injected DLL filename")

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(ENABLE_CONSOLE "Create patch console window" ON)

if (NOT ${BUILD_SHARED_LIBS})
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


############
## LOADER ##
############

add_executable(${TARGET_LOADER}
  Loader.cpp
)

target_compile_definitions(${TARGET_LOADER} PRIVATE
  INJECTION_TARGET_FILENAME="${INJECTION_TARGET_FILENAME}"
  INJECTED_PATCH_FILENAME="${INJECTED_PATCH_FILENAME}"
)

set_target_properties(${TARGET_LOADER} PROPERTIES
  WIN32_EXECUTABLE TRUE
)


###########
## PATCH ##
###########

add_library(${TARGET_PATCH} MODULE
  Patch.cpp
)

if (${ENABLE_CONSOLE})
  target_compile_definitions(${TARGET_PATCH} PRIVATE
    PATCH_ENABLE_CONSOLE
  )
endif()

target_link_options(${TARGET_PATCH} PRIVATE
  /NOIMPLIB
  /NOEXP
)

