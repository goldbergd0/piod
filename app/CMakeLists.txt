add_executable(piod
    src/main.cpp
)

target_link_libraries(piod 
    PRIVATE
        cmn
        audio_processing
)

target_include_directories(piod
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)