project('lists', 'cpp',
        default_options : ['buildtype=debugoptimized'])
srcs = ['list.cpp', 'main.cpp']

threads_dep = dependency('threads', static: true)
executable('list_mutex', srcs,
  cpp_args: '-DUSE_MUTEX', dependencies: threads_dep)
executable('list_rwlock', srcs,
  cpp_args: '-DUSE_RWLOCK', dependencies: threads_dep)

urcu_dep = dependency('liburcu-qsbr', static: true)
executable('list_rcu', srcs,
  cpp_args: '-DUSE_RCU', dependencies: [threads_dep, urcu_dep])
