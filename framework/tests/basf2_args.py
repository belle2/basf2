import os

assert 0 == os.system('basf2 --info')

assert 0 == os.system('basf2 --version')

assert 0 == os.system('basf2 --help')

assert 0 == os.system('basf2 -m')

assert 0 == os.system('basf2 -m RootOutput')

# some failing things
# exact return code of system() not defined, just unsuccesful
assert 0 != os.system('basf2 -m NonExistingModule')

assert 0 != os.system('basf2 --thisdoesntexist')

assert 0 != os.system('basf2 /this/path/doesnot/exist.py')

assert 0 != os.system('basf2 --execute-path /this/path/doesnot/exist')
