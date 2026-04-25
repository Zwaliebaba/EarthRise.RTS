# EarthRise — Windows application executable.

add_executable(EarthRise
  EarthRise.cpp
  EarthRise.h
  pch.h
  pch.cpp
)

# NeuronClient PUBLIC-links GLEW / Freetype / SFML, so they propagate here.
target_link_libraries(EarthRise PRIVATE
  "$<LINK_LIBRARY:WHOLE_ARCHIVE,NeuronClient>"
  ${EARTHRISE_SFML_MAIN_LIBRARY}
)

target_precompile_headers(EarthRise PRIVATE pch.h)

add_custom_command(TARGET EarthRise POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
          "${PROJECT_SOURCE_DIR}/GameData"
          "$<TARGET_FILE_DIR:EarthRise>/GameData"
  COMMENT "Copying GameData to the EarthRise output directory"
  VERBATIM
)
