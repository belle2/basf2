import os

Import('env')

# check whether the externals dir is set
if not os.environ.has_key('BELLE2_EXTERNALS_DIR'):
    print 'Belle II externals directory is not set up.'
    print '-> Execute "setuprel" in your local release directory.'
    Exit(1)

# build options
option = os.environ['BELLE2_OPTION']
if option == 'debug':
    env['ENV']['BOOST_OPTION'] = 'variant=debug'
    env['ENV']['CXXFLAGS'] = '-g'
    env['ENV']['GEANT4_OPTION'] = '-D g4debug=y'
    env['ENV']['ROOTBUILD'] = 'debug'
    env['ENV']['EVTGEN_OPTION'] = '--enable-debug'
elif option == 'opt':
    env['ENV']['BOOST_OPTION'] = 'variant=release'
    env['ENV']['CXXFLAGS'] = '-O3'
    env['ENV']['GEANT4_OPTION'] = '-D g4debug=n'
    env['ENV']['ROOTBUILD'] = ''
    env['ENV']['EVTGEN_OPTION'] = ''

# enable OpenGL graphics if available
if env['HAS_GRAPHICS']:
    env['ENV'].Append(GEANT4_OPTION = '-D g4vis_build_openglx_driver=\'y\' -D g4vis_use_openglx=\'y\'')

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
