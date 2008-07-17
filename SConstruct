env = Environment()
env.ParseConfig('freetype-config --libs --cflags')
env['CXXFLAGS'] += ['-g', '-O0', '-Wall']
env['LIBS']     += ['jpeg']
env.Program('fontgen', ['fontgen.cpp', 'bitmap.cpp'])

# EOF #
