add_library(cmn SHARED
    src/ArgParse.cpp
    src/cmn.cpp
    src/Usb.cpp
)

target_include_directories(cmn
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBUSB_INCLUDE_DIRS}
)

target_link_libraries(cmn 
    PUBLIC
        spdlog::spdlog
        fmt::fmt
        ${LIBUSB_LIBRARIES}
)
