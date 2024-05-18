# Include the configuration
include(config.cmake)

add_subdirectory(protobuf)

# Use file(GLOB ...) to find all files in the current directory and store them in the SERVER_FILES variable
file(GLOB SERVER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/**/*" main.cpp)

# Install all files from SERVER_FILES into the include directory
install(FILES ${SERVER_FILES} DESTINATION include)

# Generate the project executable
add_executable(efficient_server ${SERVER_FILES})

# Ensure the library is built before the executable
add_dependencies(efficient_server proto-lib)

# Find the fmt library
find_package(fmt REQUIRED)

# Link the executable with the generated protobuf library
target_link_libraries(efficient_server PRIVATE proto-lib)
target_link_libraries(efficient_server PRIVATE fmt::fmt)

target_include_directories(efficient_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/epoll)
target_include_directories(efficient_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/grid)
target_include_directories(efficient_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/locker)
target_include_directories(efficient_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/logger)
target_include_directories(efficient_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/threadpool)
target_include_directories(efficient_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/protobuf)
