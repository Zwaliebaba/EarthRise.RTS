# NeuronCore — engine core static library.

earthrise_collect_sources(NEURONCORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(NeuronCore STATIC ${NEURONCORE_SOURCES})

target_include_directories(NeuronCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Precompiled header — pch.h aggregates the heavy STL surface used here.
target_precompile_headers(NeuronCore PRIVATE pch.h)
