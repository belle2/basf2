Import('env')

# wrapper to make
make_builder = Builder(action = 'cd externals; make | tee make.log')
env['BUILDERS']['make_externals'] = make_builder
externals = env.make_externals('make.log', 'Makefile')

env['CONTINUE'] = False
Return('env')
