cmake_minimum_required (VERSION 2.6)

project (WhileCompiler)

set (Test_VERSION_MAJOR 0)
set (Test_VERSION_MINOR 0)

file(GLOB SRC_FILES
    "src/*.cpp"
    "src/io/*.cpp"
    "src/lang/*.cpp"
)

add_definitions(-std=c++11)
add_executable (WhileCompilerMain ${SRC_FILES})
target_link_libraries (WhileCompilerMain)

# copy tests to binary folder
# file( COPY "${PROJECT_SOURCE_DIR}/tests/" DESTINATION "${PROJECT_BINARY_DIR}/tests/")
