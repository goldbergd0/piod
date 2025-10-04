add_library(cmn SHARED
    src/ArgParse.cpp
    src/cmn.cpp
)

target_include_directories(cmn
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cmn 
    PUBLIC
        spdlog::spdlog
        fmt::fmt
)
