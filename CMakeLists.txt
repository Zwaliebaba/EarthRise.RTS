# EarthRise — top-level build configuration.
cmake_minimum_required(VERSION 3.25)

# --- vcpkg toolchain bootstrap (must happen before project()) -----------------
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
  set(_vcpkg_toolchain "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  if (EXISTS "${_vcpkg_toolchain}")
    set(CMAKE_TOOLCHAIN_FILE "${_vcpkg_toolchain}"
        CACHE STRING "Vcpkg toolchain file")
  endif()
  unset(_vcpkg_toolchain)
endif()

# --- MSVC Hot Reload ----------------------------------------------------------
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(EarthRise CXX)

# --- Global language + build-speed defaults -----------------------------------
# C++ "latest": rely on MSVC's /std:c++latest rather than CMake's cxx_std_NN
# feature mapping (CMake 4.3 does not yet know that MSVC 14.50 supports C++26).
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
  add_compile_options(
    /std:c++latest   # C++26 preview on MSVC 14.50+
    /MP              # Per-target parallel compilation — large build-speed win.
    /Zc:__cplusplus  # Report the real value of __cplusplus.
    /Zc:preprocessor # Conformant preprocessor.
    /permissive-     # Strict conformance.
  )
else()
  set(CMAKE_CXX_STANDARD          23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# --- Third-party packages (resolved once, consumed by sub-projects) -----------
find_package(GLEW                  REQUIRED)
find_package(Freetype              REQUIRED)
find_package(SFML 3 CONFIG REQUIRED COMPONENTS System Window Graphics Network)

# sfml-main is shipped as a plain static library (not a CMake target).
find_library(EARTHRISE_SFML_MAIN_LIBRARY
  NAMES sfml-main sfml-main-d
  HINTS
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib"
  REQUIRED
)

# --- Source-collection helper used by sub-projects ----------------------------
# Recursively globs C/C++ sources and headers under ROOT, skipping build/IDE
# output directories that may sit next to the source tree.
function(earthrise_collect_sources OUT_VAR ROOT)
  file(GLOB_RECURSE _sources CONFIGURE_DEPENDS
    "${ROOT}/*.c"   "${ROOT}/*.cc"  "${ROOT}/*.cpp" "${ROOT}/*.cxx"
    "${ROOT}/*.h"   "${ROOT}/*.hh"  "${ROOT}/*.hpp" "${ROOT}/*.hxx"
  )
  set(_filtered)
  foreach(_path IN LISTS _sources)
    string(REPLACE "\\" "/" _path "${_path}")
    if (NOT _path MATCHES "/(x64|x86|Debug|Release|RelWithDebInfo|MinSizeRel|out|\\.vs|Generated Files|generated|winrt|tlog|embed|microsoft)(/|$)")
      list(APPEND _filtered "${_path}")
    endif()
  endforeach()
  set(${OUT_VAR} "${_filtered}" PARENT_SCOPE)
endfunction()

# --- Sub-projects -------------------------------------------------------------
add_subdirectory(NeuronCore)
add_subdirectory(NeuronClient)
add_subdirectory(EarthRise)
