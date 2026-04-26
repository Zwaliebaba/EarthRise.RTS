# NeuronClient — gameplay/UI/rendering static library.

earthrise_collect_sources(NEURONCLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE _neuronclient_include_entries CONFIGURE_DEPENDS LIST_DIRECTORIES TRUE
  "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(_path IN LISTS _neuronclient_include_entries)
  string(REPLACE "\\" "/" _path "${_path}")
  if (IS_DIRECTORY "${_path}"
      AND NOT _path STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/Audio"
      AND NOT _path STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/LTE"
      AND NOT _path MATCHES "/(x64|x86|Debug|Release|RelWithDebInfo|MinSizeRel|out|\.vs|Generated Files|generated|winrt|tlog|embed|microsoft)(/|$)")
    list(APPEND NEURONCLIENT_INCLUDE_DIRECTORIES "${_path}")
  endif()
endforeach()
list(REMOVE_DUPLICATES NEURONCLIENT_INCLUDE_DIRECTORIES)

add_library(NeuronClient STATIC ${NEURONCLIENT_SOURCES})

target_include_directories(NeuronClient PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(NeuronClient SYSTEM PUBLIC ${NEURONCLIENT_INCLUDE_DIRECTORIES})

# PUBLIC: include dirs and link requirements propagate to consumers (EarthRise).
target_link_libraries(NeuronClient PUBLIC
  NeuronCore
  GLEW::GLEW
  Freetype::Freetype
  SFML::System
  SFML::Window
  SFML::Graphics
  SFML::Network
)

if (WIN32)
  target_link_libraries(NeuronClient PUBLIC
    ole32
    xaudio2
  )
endif()

if (WIN32 AND EARTHRISE_ENABLE_D3D12)
  target_link_libraries(NeuronClient PUBLIC
    d3d12
    dxgi
    dxguid
    user32
  )

  target_compile_definitions(NeuronClient PUBLIC EARTHRISE_ENABLE_D3D12=1)

  set(EARTHRISE_D3D12_SHADER_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/LTE/Render/D3D12/Shaders/BootstrapClear.hlsl")

  if (EARTHRISE_DXC_EXECUTABLE AND EXISTS "${EARTHRISE_D3D12_SHADER_SOURCE}")
    set(EARTHRISE_D3D12_GENERATED_SHADER_DIR
      "${CMAKE_CURRENT_BINARY_DIR}/Generated/Shaders")
    set(EARTHRISE_D3D12_BOOTSTRAP_VS
      "${EARTHRISE_D3D12_GENERATED_SHADER_DIR}/BootstrapClearVS.h")
    set(EARTHRISE_D3D12_BOOTSTRAP_PS
      "${EARTHRISE_D3D12_GENERATED_SHADER_DIR}/BootstrapClearPS.h")

    add_custom_command(
      OUTPUT "${EARTHRISE_D3D12_BOOTSTRAP_VS}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${EARTHRISE_D3D12_GENERATED_SHADER_DIR}"
      COMMAND "${EARTHRISE_DXC_EXECUTABLE}"
        -T vs_6_0 -E VSMain
        -Fh "${EARTHRISE_D3D12_BOOTSTRAP_VS}"
        -Vn g_BootstrapClearVS
        "${EARTHRISE_D3D12_SHADER_SOURCE}"
      DEPENDS "${EARTHRISE_D3D12_SHADER_SOURCE}"
      VERBATIM)

    add_custom_command(
      OUTPUT "${EARTHRISE_D3D12_BOOTSTRAP_PS}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${EARTHRISE_D3D12_GENERATED_SHADER_DIR}"
      COMMAND "${EARTHRISE_DXC_EXECUTABLE}"
        -T ps_6_0 -E PSMain
        -Fh "${EARTHRISE_D3D12_BOOTSTRAP_PS}"
        -Vn g_BootstrapClearPS
        "${EARTHRISE_D3D12_SHADER_SOURCE}"
      DEPENDS "${EARTHRISE_D3D12_SHADER_SOURCE}"
      VERBATIM)

    add_custom_target(NeuronClientD3D12Shaders
      DEPENDS
        "${EARTHRISE_D3D12_BOOTSTRAP_VS}"
        "${EARTHRISE_D3D12_BOOTSTRAP_PS}")
    add_dependencies(NeuronClient NeuronClientD3D12Shaders)

    target_include_directories(NeuronClient PRIVATE
      "${EARTHRISE_D3D12_GENERATED_SHADER_DIR}")
    target_compile_definitions(NeuronClient PRIVATE
      EARTHRISE_D3D12_GENERATED_SHADERS=1)
  else()
    target_compile_definitions(NeuronClient PRIVATE
      EARTHRISE_D3D12_GENERATED_SHADERS=0)
  endif()
else()
  target_compile_definitions(NeuronClient PUBLIC EARTHRISE_ENABLE_D3D12=0)
endif()

target_precompile_headers(NeuronClient PRIVATE pch.h)
