# Microsoft Native Unit Test projects for the three active modules.

if (NOT MSVC)
  message(STATUS "Microsoft Native Unit Test projects are only enabled for MSVC builds.")
  return()
endif()

set(_earthrise_vs_install_path "${CMAKE_VS_INSTALLATION_PATH}")
if (NOT _earthrise_vs_install_path AND DEFINED ENV{VSINSTALLDIR})
  file(TO_CMAKE_PATH "$ENV{VSINSTALLDIR}" _earthrise_vs_install_path)
endif()
if (NOT _earthrise_vs_install_path)
  set(_earthrise_vs_install_path "C:/Program Files/Microsoft Visual Studio/18/Community")
endif()

set(EARTHRISE_MS_NATIVE_TEST_INCLUDE_DIR
  "${_earthrise_vs_install_path}/VC/Auxiliary/VS/UnitTest/include")

if (CMAKE_VS_PLATFORM_NAME MATCHES "ARM64")
  set(_earthrise_unit_test_arch "ARM64")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_earthrise_unit_test_arch "x64")
else()
  set(_earthrise_unit_test_arch "x86")
endif()

set(EARTHRISE_MS_NATIVE_TEST_LIBRARY
  "${_earthrise_vs_install_path}/VC/Auxiliary/VS/UnitTest/lib/${_earthrise_unit_test_arch}/Microsoft.VisualStudio.TestTools.CppUnitTestFramework.lib")
set(EARTHRISE_VSTEST_CONSOLE
  "${_earthrise_vs_install_path}/Common7/IDE/CommonExtensions/Microsoft/TestWindow/vstest.console.exe")

if (NOT EXISTS "${EARTHRISE_MS_NATIVE_TEST_INCLUDE_DIR}/CppUnitTest.h")
  message(FATAL_ERROR "Could not find CppUnitTest.h at ${EARTHRISE_MS_NATIVE_TEST_INCLUDE_DIR}")
endif()
if (NOT EXISTS "${EARTHRISE_MS_NATIVE_TEST_LIBRARY}")
  message(FATAL_ERROR "Could not find Microsoft Native Unit Test library at ${EARTHRISE_MS_NATIVE_TEST_LIBRARY}")
endif()

function(earthrise_add_microsoft_native_test TARGET_NAME)
  set(_multi_value_args SOURCES LIBRARIES DEPENDS INCLUDES CTEST_CATEGORIES)
  cmake_parse_arguments(EARTHRISE_TEST "" "" "${_multi_value_args}" ${ARGN})

  add_library(${TARGET_NAME} SHARED ${EARTHRISE_TEST_SOURCES})
  target_include_directories(${TARGET_NAME} SYSTEM PRIVATE
    "${EARTHRISE_MS_NATIVE_TEST_INCLUDE_DIR}")
  target_include_directories(${TARGET_NAME} PRIVATE
    ${EARTHRISE_TEST_INCLUDES})
  target_link_libraries(${TARGET_NAME} PRIVATE
    "${EARTHRISE_MS_NATIVE_TEST_LIBRARY}"
    ${EARTHRISE_TEST_LIBRARIES})
  target_compile_options(${TARGET_NAME} PRIVATE
    /W4
    /WX
    /wd4100
    /wd4189
    /wd4244
    /wd4245
    /wd4267
    /wd4305
    /wd4456
    /wd4457
    /wd4458
    /wd4459
    /wd4701
    /wd4702
    /wd4834)
  target_compile_definitions(${TARGET_NAME} PRIVATE
    EARTHRISE_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

  if (EARTHRISE_TEST_DEPENDS)
    add_dependencies(${TARGET_NAME} ${EARTHRISE_TEST_DEPENDS})
  endif()

  set_target_properties(${TARGET_NAME} PROPERTIES
    FOLDER "Tests"
    VS_GLOBAL_ProjectSubType "NativeUnitTestProject")

  if (EXISTS "${EARTHRISE_VSTEST_CONSOLE}")
    if (EARTHRISE_TEST_CTEST_CATEGORIES)
      string(REGEX REPLACE "\\.Test$" "" _test_name_prefix "${TARGET_NAME}")
      foreach(_category IN LISTS EARTHRISE_TEST_CTEST_CATEGORIES)
        add_test(NAME ${_test_name_prefix}.${_category}
          COMMAND "${EARTHRISE_VSTEST_CONSOLE}" "$<TARGET_FILE:${TARGET_NAME}>" "/TestCaseFilter:Category=${_category}")
      endforeach()
    else()
      add_test(NAME ${TARGET_NAME}
        COMMAND "${EARTHRISE_VSTEST_CONSOLE}" "$<TARGET_FILE:${TARGET_NAME}>")
    endif()
  endif()
endfunction()

earthrise_add_microsoft_native_test(NeuronCore.Test
  SOURCES
    NeuronCore.Test/Unit/AsyncLoaderTests.cpp
    NeuronCore.Test/Unit/DataSerializationTests.cpp
    NeuronCore.Test/Unit/DebugTests.cpp
    NeuronCore.Test/Unit/EventManagerTests.cpp
    NeuronCore.Test/Unit/MathAndTimerTests.cpp
    NeuronCore.Test/Unit/NeuronCoreSmokeTests.cpp
    NeuronCore.Test/Unit/TasksThreadTests.cpp
    NeuronCore.Test/Integration/FileSysIntegrationTests.cpp
    NeuronCore.Test/Integration/NeuronCoreIntegrationTests.cpp
  LIBRARIES
    NeuronCore
  CTEST_CATEGORIES
    Unit.AsyncLoader
    Unit.Serialization
    Unit.Diagnostics
    Unit.Events
    Unit.MathAndTimer
    Unit.Threading
    Unit.Smoke
    Integration.FileSystem
    Integration.CoreRuntime)

earthrise_add_microsoft_native_test(NeuronClient.Test
  SOURCES
    NeuronClient.Test/Unit/NeuronClientSmokeTests.cpp
    NeuronClient.Test/Integration/NeuronClientIntegrationTests.cpp
  LIBRARIES
    NeuronClient)

earthrise_add_microsoft_native_test(EarthRise.Test
  SOURCES
    EarthRise.Test/Unit/EarthRiseSmokeTests.cpp
    EarthRise.Test/Integration/EarthRiseIntegrationTests.cpp
  INCLUDES
    "${CMAKE_SOURCE_DIR}/EarthRise"
  DEPENDS
    EarthRise)
