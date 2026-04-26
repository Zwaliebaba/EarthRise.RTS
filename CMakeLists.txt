# EarthRise — top-level build configuration.
cmake_minimum_required(VERSION 3.28)

# --- MSVC Hot Reload ----------------------------------------------------------
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(Earthrise LANGUAGES CXX RC)

include(CTest)

# ── C++ standard ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ── Static CRT linkage (/MT, /MTd) ─────────────────────────────────────────
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_compile_definitions(UNICODE _UNICODE)

# ── Common compiler flags ───────────────────────────────────────────────────
add_compile_options(
    /W4             # Warning level 4
    /sdl            # Security Development Lifecycle checks
    /permissive-    # Standards conformance
    /bigobj         # Large object file support
    /utf-8          # Source and execution character set
    /Zc:__cplusplus # Report correct __cplusplus macro value
)

# ── Release optimisations ───────────────────────────────────────────────────
add_compile_options(
    $<$<CONFIG:Release>:/GL>   # Whole-program optimisation
    $<$<CONFIG:Release>:/Oi>   # Intrinsic functions
    $<$<CONFIG:Release>:/Gy>   # Function-level linking
)

add_link_options(
    $<$<CONFIG:Release>:/LTCG>         # Link-time code generation
    $<$<CONFIG:Release>:/OPT:REF>      # Eliminate unreferenced code
    $<$<CONFIG:Release>:/OPT:ICF>      # Identical COMDAT folding
)

# ── Unicode ─────────────────────────────────────────────────────────────────
add_compile_definitions(UNICODE _UNICODE)

# ── Debug / Release defines ────────────────────────────────────────────────
add_compile_definitions(
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

# ── C++/WinRT (via vcpkg) ───────────────────────────────────────────────────
find_package(cppwinrt CONFIG REQUIRED)

# ── Shader compilation (DXC → header) ──────────────────────────────────────
include(${CMAKE_SOURCE_DIR}/cmake/CompileShaders.cmake)

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
    "${ROOT}/*.cpp" "${ROOT}/*.h" 
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
