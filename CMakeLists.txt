cmake_minimum_required(VERSION 3.10.0)
project(piod VERSION 0.1.0 LANGUAGES C CXX)

# Options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
target_link_libraries(spdlog::spdlog INTERFACE fmt::fmt)
# find_package(FFTW3 REQUIRED COMPONENTS single)
find_library(FFTW3F_LIBRARY NAMES fftw3f)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

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