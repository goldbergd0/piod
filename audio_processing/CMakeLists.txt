add_library(audio_processing SHARED
    src/audio_processing.cpp
)

target_include_directories(audio_processing
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)