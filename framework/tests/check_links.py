# checks all built shared libraries for undefined symbols
# (not satisfied by links)

# this catches problems that are not apparent when creating another
# .so with them, but only when linking executables

import os
import sys
import tempfile

if len(sys.argv) < 2:
    print "Test disabled, start with - --really argument to run it manually."
    sys.exit(0)

localdir = os.environ['BELLE2_LOCAL_DIR']
subdir = os.environ['BELLE2_SUBDIR']

libdir = localdir + '/lib/' + subdir

#define main() for building executables
maincc = tempfile.mkstemp(suffix='.cc')
mainccfd = open(maincc[1], 'w+')
mainccfd.write("int main() { return 0; }\n")
mainccfd.close()

libs = os.listdir(libdir)
for lib in libs:
    if not lib.endswith('.so'):
        continue  # not a shared object

    #try linking an executable with the given library
    tmpfile = tempfile.NamedTemporaryFile()
    try:
        os.system("g++ -o %s -L%s -l:%s %s" %
                  (tmpfile.name, libdir, lib, maincc[1]))
        tmpfile.close()  # might not work
    except:
        pass

os.unlink(maincc[1])
