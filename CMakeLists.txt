cmake_minimum_required(VERSION 3.25)
project(semestral)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(protobuf)
include_directories(src)
include_directories(src/data)
include_directories(src/epoll)
include_directories(src/logging)
include_directories(src/protobuf)
include_directories(src/threadpool)

add_executable(semestral
        protobuf/scheme.proto
        src/data/DataGrid.hh
        src/data/DataGridLogging.cpp
        src/data/DataGridInsertion.cpp
        src/data/DataGridPath.cpp
        src/data/DataGridProtobuf.cpp
        src/epoll/EpollConnection.cpp
        src/epoll/EpollConnection.hh
        src/epoll/EpollEntry.cpp
        src/epoll/EpollEntry.hh
        src/epoll/EpollInstance.cpp
        src/epoll/EpollInstance.hh
        src/epoll/EpollSocket.cpp
        src/epoll/EpollSocket.hh
        src/logging/logger.cpp
        src/logging/logger.hh
        src/protobuf/scheme.pb.cc
        src/protobuf/scheme.pb.h
        src/threadpool/ThreadPool.cpp
        src/threadpool/ThreadPool.hh
        src/main.cpp
        src/meson.build
        meson.build
        shell.nix src/data/DataGridUtil.cpp)
