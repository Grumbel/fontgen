env = Environment()
env.ParseConfig('freetype-config --libs --cflags')
env['CXXFLAGS'] += ['-g', '-O0', '-Wall']
env['CPPPATH'] += ['-I../pingus/src/']
env['LIBS']     += ['jpeg']
env.Program('fontgen', ['fontgen.cpp', 'bitmap.cpp', '../pingus/src/utf8_iterator.cpp'])

# EOF #
