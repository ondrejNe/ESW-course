boost_dep = dependency('boost', modules : ['filesystem'])
opencv_dep = dependency('opencv4')
srcs = ['find_ellipse.cpp', 'ellipse.cpp', 'imgproc.cpp']
executable('find_ellipse', srcs, dependencies: [boost_dep, opencv_dep])
