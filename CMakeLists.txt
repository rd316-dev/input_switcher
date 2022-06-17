cmake_minimum_required(VERSION 3.23)
project(input_switcher)
set(CMAKE_CXX_STANDARD 17)

#add_compile_options("$<$<C_COMPILER_ID:mingw>:/utf-8>")
#add_compile_options("$<$<CXX_COMPILER_ID:mingw>:/utf-8>")

#set(WIN32 True)

include_directories("src/")

set(SOURCE_FILES
	"src/main.cpp"
)

add_executable(input_switcher WIN32 ${SOURCE_FILES})