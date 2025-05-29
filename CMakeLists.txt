cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)


set(TARGET FA18E_SH_patch)
project(${TARGET} VERSION 1.0 LANGUAGES CXX ASM_MASM)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message(FATAL_ERROR "Only MSVC compiler is supported")
endif()

set(TARGET_LOADER F18loader)
set(TARGET_PATCH F18patch)

set(INJECTION_TARGET_FILENAME "F18.EXE" CACHE STRING "Injection target filename")
set(INJECTION_TARGET_MD5 "3E9301D75C8D3B212AA18C9A88CD06BF" CACHE STRING "Injection target MD5 value")
set(INJECTED_PATCH_FILENAME "F18patch.dll" CACHE STRING "Injected DLL filename")

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(ENABLE_CONSOLE "Create patch console window" OFF)

if (NOT ${BUILD_SHARED_LIBS})
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


############
## LOADER ##
############

add_executable(${TARGET_LOADER}
  src/Loader.cpp
)

target_compile_definitions(${TARGET_LOADER} PRIVATE
  WIN32_LEAN_AND_MEAN
  INJECTION_TARGET_FILENAME="${INJECTION_TARGET_FILENAME}"
  INJECTION_TARGET_MD5="${INJECTION_TARGET_MD5}"
  INJECTED_PATCH_FILENAME="${INJECTED_PATCH_FILENAME}"
)

target_link_libraries(${TARGET_LOADER} PRIVATE
  Crypt32
)

set_target_properties(${TARGET_LOADER} PROPERTIES
  WIN32_EXECUTABLE TRUE
)


###########
## PATCH ##
###########

add_library(${TARGET_PATCH} MODULE
  src/Patch.cpp
  src/PatchAPI.cpp
)

target_compile_definitions(${TARGET_PATCH} PRIVATE
  WIN32_LEAN_AND_MEAN
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
