# Find required protobuf package
find_package(Protobuf REQUIRED)

# Include generated *.pb.h files
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Proto file
set(PROTO_FILE scheme.proto)

# Generated sources
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

# Create a library from the generated protobuf files
add_library(proto-lib ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(proto-lib ${PROTOBUF_LIBRARY})
