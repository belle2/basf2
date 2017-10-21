#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################################
# Split one big data file to smaller files.
# command: basf2 split_file.py filename entries_per_file nfile
###############################################################################

from basf2 import *
from ROOT import Belle2
argvs = sys.argv
run = argvs[1]
nevent = int(argvs[2])
nfile = int(argvs[3])

Dir = 'GCR2017_unpacked/'


class Split(Module):
    """
    Class to split one file to multi files with small data size.
    """

    def event(self):
        """reimplementation of Module::event()."""
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        if not evtmetadata:
            B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            self.return_value(event // nevent)


out = []
main = create_path()
main.add_module('RootInput', inputFileName=Dir + run + '.root')
main.add_module('ProgressBar')
split = Split()
main.add_module(split)
for i in range(0, int(nfile)):
    out.append(create_path())
    out[i].add_module('RootOutput', outputFileName=Dir + run + '_' + str(i) + '.root')
    if i == nfile - 1:
        split.if_value('>={}'.format(i), out[i])
    else:
        split.if_value('={}'.format(i), out[i])

process(main)

print(statistics)
