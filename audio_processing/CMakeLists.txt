add_library(audio_processing SHARED
    src/audio_processing.cpp
    src/AudioListener.cpp
    src/AudioProcess.cpp
    src/AudioDrawer.cpp
)


target_include_directories(audio_processing 
    PRIVATE
        ${ALSA_INCLUDE_DIRS}
        ${FFTW3F_INCLUDE_DIRS}
)
target_link_libraries(audio_processing
    PRIVATE
        ${ALSA_LIBRARIES}
        cmn
        ${FFTW3F_LIBRARY}
)

target_include_directories(audio_processing
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)