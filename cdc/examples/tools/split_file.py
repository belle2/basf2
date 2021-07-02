#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

###############################################################################
# Split one big data file to smaller files.
# command: basf2 split_file.py filename entries_per_file nfile
###############################################################################

import basf2 as b2
from ROOT import Belle2
import sys
argvs = sys.argv
run = argvs[1]
nevent = int(argvs[2])
nfile = int(argvs[3])

Dir = 'GCR2017_unpacked/'


class Split(b2.Module):
    """
    Class to split one file to multi files with small data size.
    """

    def event(self):
        """reimplementation of Module::event()."""
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        if not evtmetadata:
            b2.B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            self.return_value(event // nevent)


out = []
main = b2.create_path()
main.add_module('RootInput', inputFileName=Dir + run + '.root')
main.add_module('ProgressBar')
split = Split()
main.add_module(split)
for i in range(0, int(nfile)):
    out.append(b2.create_path())
    out[i].add_module('RootOutput', outputFileName=Dir + run + '_' + str(i) + '.root')
    if i == nfile - 1:
        split.if_value('>={}'.format(i), out[i])
    else:
        split.if_value('={}'.format(i), out[i])

b2.process(main)

print(b2.statistics)
