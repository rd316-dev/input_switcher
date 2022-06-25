cmake_minimum_required(VERSION 3.23)
project(input_switcher)
set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
    "src/main.cpp"
)

add_executable(input_switcher WIN32 ${SOURCE_FILES})

target_include_directories(input_switcher PRIVATE "src/")
target_link_libraries(input_switcher "uxtheme.lib" "-static-libgcc" "-static-libstdc++")