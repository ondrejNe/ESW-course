# Dependencies
boost_regex_dep = meson.get_compiler('cpp').find_library('boost_regex')
thread_dep = dependency('threads')
protobuf_dep = dependency('protobuf')
jsoncpp_dep = dependency('jsoncpp')

# Generate files
protoc = find_program('protoc', required : true)
gen = generator(protoc,
                output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
                arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])
generated = gen.process('measurements.proto')

# All sources
srcs = [
  'dataset.cpp',
  'main.cpp',
  'measurementinfo.cpp',
  'result.cpp',
  generated,
]

# Server executable
executable(
  'server',
  srcs,
  link_args: [
    '-lavrocpp'
  ],
  dependencies: [
    thread_dep,
    boost_regex_dep,
    jsoncpp_dep,
    protobuf_dep,
  ]
)
