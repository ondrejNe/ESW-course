# Dependencies
boost_regex_dep = meson.get_compiler('cpp').find_library('boost_regex')
thread_dep = dependency('threads')
protobuf_dep = dependency('protobuf')
jsoncpp_dep = dependency('jsoncpp')

# All sources
srcs = [
  'dataset.cpp',
  'main.cpp',
  'measurementinfo.cpp',
  'result.cpp',
  'measurements.pb.cc'
]

# Specify libavrocpp library and include paths
avrocpp_lib_dir = '/nix/store/0zihsc1dzpbdzidm052zl765hg3rc3hz-avro-c++-1.11.0/lib/'
avrocpp_include_dir = '/nix/store/0zihsc1dzpbdzidm052zl765hg3rc3hz-avro-c++-1.11.0/include/'

# Server executable
executable(
  'server',
  srcs,
  include_directories: [
    avrocpp_include_dir,
  ],
  link_args: [
    '-L/usr/local/lib/',
    '-lavrocpp_s'
  ],
  dependencies: [
    thread_dep,
    boost_regex_dep,
    jsoncpp_dep,
    protobuf_dep,
  ]
)
