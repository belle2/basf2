import os

Import('env')

# wrapper to make
make_builder = Builder(action = 'cd externals; make | tee make.log')
env['BUILDERS']['make_externals'] = make_builder
externals = env.make_externals('make.log', 'Makefile')

# google test
env.Install(os.path.join(env['EXTINCDIR'], 'gtest'),
            Glob(os.path.join(env['EXTDIR'], 'gtest', 'include', 'gtest', '*.h')))
env.Install(os.path.join(env['EXTINCDIR'], 'gtest', 'internal'),
            Glob(os.path.join(env['EXTDIR'], 'gtest', 'include', 'gtest', 'internal', '*.h')))
env.StaticLibrary(os.path.join(env['EXTLIBDIR'], 'gtest'),
                  [os.path.join(env['EXTDIR'], 'gtest', 'src', 'gtest-all.cc'),
                  os.path.join(env['EXTDIR'], 'gtest', 'src', 'gtest_main.cc')],
                  CPPPATH=[os.path.join(env['EXTDIR'], 'gtest')] + env['CPPPATH'])

env['CONTINUE'] = False
Return('env')
