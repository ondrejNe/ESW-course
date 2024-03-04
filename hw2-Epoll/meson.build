project('esw-epoll-cc', 'cpp',
  default_options: [
    'warning_level=3',
    'werror=true',
    'optimization=2',
    'debug=true'
])
src_files = [
  'main.cc', 'EpollEntry.cc', 'EpollInstance.cc',
  'EpollStdIn.cc', 'EpollTimer.cc', 'EpollConnection.cc',
  'EpollSocket.cc'
]
executable('epoll_server', src_files)
test('first run', find_program('t.sh'), is_parallel : false)
test('second run', find_program('t.sh'), is_parallel : false)
