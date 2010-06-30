import os

Import('env')

# wrapper to make
make_builder = Builder(action = 'cd externals; make > make.log')
env['BUILDERS']['make_externals'] = make_builder
externals = env.make_externals('make.log', 'Makefile')
#env.Alias('externals', externals)

env['CONTINUE'] = False
Return('env')
