#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2.core import basf2label, basf2copyright, basf2license, list_module_search_paths, LogPythonInterface
from basf2.version import version, release
from basf2 import conditions

import os
import sys

# The Belle II logo graphics
logo = """\

                      eeeeeeee
                eeeeeeeeeeeeeeeeeeee
          eeeeeeee        eeeeeeeeeeeeeeee
        eeeeee           eeee         eeeeee
       eeee              eeee           eeee
        eeee            eeee           eeee
         eeeee          eeee          eeee
            eeee       eeeeeeeeeeeeeeeee
                       eeeeeeeeeeeee

                      eeeeeeeeeeeeee
                      eeeeeeeeeeeeeeeeeee
       eeee          eeee            eeeeee
     eeee            eeee             eeeeee
   eeee             eeee                eeeee
  eeee              eeee                eeeee
    eeee           eeee             eeeeeee
      eeeeee       eeeeeeeeeeeeeeeeeeeee
         eeeeeeeeeeeeeeeeeeeeeeeeee
             eeeeeeeeeeeeeeeee

   BBBBBBB             ll ll          2222222
   BB    BB    eeee    ll ll   eeee    22 22
   BB    BB   ee   ee  ll ll  ee   ee  22 22
   BBBBBBB   eeeeeeee  ll ll eeeeeeee  22 22
   BB    BB  ee        ll ll ee        22 22
   BB     BB  ee   ee  ll ll  ee   ee  22 22
   BBBBBBBB    eeeee   ll ll   eeeee  2222222

"""

if LogPythonInterface.terminal_supports_colors():
    CSI = "\x1B["
    color = CSI + '93;44m'
    reset = CSI + '0m'
else:
    color = ""
    reset = ""

for line in logo.splitlines():
    print(color + line.ljust(48) + reset)

print('')
print(basf2label.center(48))
print(basf2copyright.center(48))
print(basf2license.center(48))
print(('Release ' + release).center(48))
print(('Version ' + version).center(48))
print('')
print('-' * 48)
for var in ["RELEASE", "RELEASE_DIR", "LOCAL_DIR", "SUBDIR", "EXTERNALS_VERSION", "ARCH"]:
    name = "BELLE2_" + var
    print((name + ":").ljust(25), os.environ.get(name, ''))

print('Default global tags:'.ljust(25), conditions.default_globaltags)
print('Kernel version:'.ljust(25), os.uname()[2])
python_version = sys.version_info[:3]
print('Python version:'.ljust(25), '.'.join(str(ver) for ver in python_version))
try:
    from ROOT import gROOT
    gROOT.SetBatch()
    rootver = gROOT.GetVersion()
except ImportError:
    rootver = 'PyROOT broken, cannot get version!'
print('ROOT version:'.ljust(25), rootver)
print('')
print('basf2 module directories:'.ljust(25))
for dirname in list_module_search_paths():
    print(' ', dirname)

print('-' * 48)
