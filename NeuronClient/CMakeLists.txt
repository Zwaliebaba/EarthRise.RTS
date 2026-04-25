# NeuronClient — gameplay/UI/rendering static library.

earthrise_collect_sources(NEURONCLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(NeuronClient STATIC ${NEURONCLIENT_SOURCES})

target_include_directories(NeuronClient PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

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

target_precompile_headers(NeuronClient PRIVATE pch.h)
