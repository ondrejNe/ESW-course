# Dependencies
thread_dep = dependency('threads')
protobuf_dep = dependency('protobuf')
boost_regex_dep = meson.get_compiler('cpp').find_library('boost_regex')

# All sources
srcs = [

  'data/DataGridLogging.cpp',
  'data/DataGridInsertion.cpp',
  'data/DataGridPath.cpp',
  'data/DataGridProtobuf.cpp',
  'data/DataGridUtil.cpp',

  'epoll/EpollConnection.cpp',
  'epoll/EpollEntry.cpp',
  'epoll/EpollInstance.cpp',
  'epoll/EpollSocket.cpp',

  'protobuf/scheme.pb.cc',

  'threadpool/ThreadPool.cpp',
  
  'main.cpp',
]

# Server executable
executable(
  'server', 
  srcs,
  dependencies: [
    thread_dep, 
    protobuf_dep,
    boost_regex_dep 
  ],
)
