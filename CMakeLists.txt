cmake_minimum_required(VERSION 3.10.0)
project(piod VERSION 0.1.0 LANGUAGES C CXX)

# Options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/export)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/export)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/export)

add_subdirectory(audio_processing)
add_subdirectory(cmn)
add_subdirectory(app)


# include(ExternalProject)

# # Download and build PortAudio
# ExternalProject_Add(portaudio
#     GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
#     GIT_TAG master
#     UPDATE_COMMAND ""
#     CONFIGURE_COMMAND ./configure --without-jack
#     BUILD_COMMAND make
#     INSTALL_COMMAND make install
#     BUILD_IN_SOURCE 1
# )