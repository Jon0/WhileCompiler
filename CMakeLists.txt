cmake_minimum_required (VERSION 2.6)

project (WhileCompiler)

set (Test_VERSION_MAJOR 0)
set (Test_VERSION_MINOR 0)

file(GLOB SRC_FILES
    "src/*.cpp"
    "src/io/*.cpp"
    "src/java/*.cpp"
    "src/lang/*.cpp"
    "src/test/*.cpp"
    "src/utils/*.cpp"
    "src/x86/*.cpp"
)

add_definitions(-std=c++11)
add_executable (WhileCompilerMain ${SRC_FILES})
target_link_libraries (WhileCompilerMain)

# copy library to binary folder
file( COPY "${PROJECT_SOURCE_DIR}/lib/" DESTINATION "${PROJECT_BINARY_DIR}/lib/")

# copy tests to binary folder
# file( COPY "${PROJECT_SOURCE_DIR}/tests/" DESTINATION "${PROJECT_BINARY_DIR}/tests/")
