#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# checks all built shared libraries for undefined symbols
# (not satisfied by links)

# this catches problems that are not apparent when creating another
# .so with them, but only when linking executables

import os
import sys
import tempfile

# define main() for building executables
maincc = tempfile.mkstemp(suffix='.cc')
mainccfd = open(maincc[1], 'w+')
mainccfd.write("int main() { return 0; }\n")
mainccfd.close()


def checkLinks(subdir):
    """
    Run check on all .so files in given subdirectory (e.g lib, modules)
    """
    belle2_localdir = os.environ['BELLE2_LOCAL_DIR']
    belle2_subdir = os.environ['BELLE2_SUBDIR']

    global maincc
    libdir = belle2_localdir + '/' + subdir + '/' + belle2_subdir
    libs = os.listdir(libdir)
    for lib in libs:
        if not lib.endswith('.so'):
            continue  # not a shared object

        # try linking an executable with the given library
        tmpfile = tempfile.NamedTemporaryFile()
        try:
            os.system("g++ -o %s -L%s -l:%s %s" %
                      (tmpfile.name, libdir, lib, maincc[1]))
            tmpfile.close()  # might not work
            print("passed: " + lib)
        except:
            print("FAILED: " + lib)
            pass

checkLinks('lib')
checkLinks('modules')

os.unlink(maincc[1])
