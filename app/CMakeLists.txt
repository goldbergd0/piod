add_executable(piod
    main.cpp
)

target_link_libraries(piod PRIVATE audio_processing)