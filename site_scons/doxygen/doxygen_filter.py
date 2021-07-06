#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys
import os
import re


image_path = 'build/module_io'
files = []
if os.path.isdir(image_path):
    files = os.listdir(image_path)

# list of modules with I/O plots, _without_ "Module" suffix
modules_with_plots = [f[:-4] for f in files if re.match(r'.*.png', f) and os.stat(os.path.join(image_path, f)).st_size > 0]

# corresponding header files, in lower case
headers = [m.lower() + 'module.h' for m in modules_with_plots]

filename = sys.argv[1]
subdirs = os.path.relpath(filename).split(os.path.sep)
group = subdirs[0]
if 'modules' in subdirs:
    group += '_modules'
elif 'dataobjects' in subdirs:
    group += '_dataobjects'

found_idx = None
try:
    found_idx = headers.index(os.path.basename(filename).lower())
    # ok, we've got a header with corresponding plot
    module = modules_with_plots[found_idx]
    classname = module + 'Module'
except Exception:
    pass

belle2ns = False
for line in open(filename):

    # python comments
    try:
        line = line.replace('#: ', '## ')
    except IOError:
        pass  # doxygen closes pipe for whatever reason

    # module io plot
    if found_idx is not None:
        # TODO: also allow other whitespace after class?
        if re.match(r'.*class ' + classname + '.*', line):
            # found class declaration, add comment before it
            print('''/**
* \\image html ''' + module + '''.png
*/''')

    # end of belle2 namespace -> end grouping
    try:
        if re.match(r'^}.*', line) and belle2ns:
            belle2ns = False
            print('  /*! @} */')
    except IOError:
        pass  # doxygen closes pipe for whatever reason

    # addtogroup
    try:
        if re.match(r'.*/\*\*.*\\addtogroup.*', line):
            print('/**')
        elif not re.match(r'.*\\addtogroup.*', line):
            sys.stdout.write(line)  # print raw, without added \n
    except IOError:
        pass  # doxygen closes pipe for whatever reason

    # beginning of belle2 namespace -> start grouping
    try:
        if re.match(r'namespace Belle2 {.*', line):
            belle2ns = True
            print("""  /**
   * @addtogroup %s
   * @{
   */""" % group)
    except IOError:
        pass  # doxygen closes pipe for whatever reason

# avoid some further errors with lost stdout/stderr
try:
    sys.stdout.close()
except Exception:
    pass
try:
    sys.stderr.close()
except Exception:
    pass
