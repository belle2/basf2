#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# steering file - 2017 Belle II Collaboration
######################################################


# we need to modify this to get data from different coppers

# 4 2Ds
# hslb = (('11000001', 'b'),
#         ('11000001', 'a'),
#         ('11000002', 'a'),
#         ('11000002', 'b'))

# 2D0 and NN

hslb = (('11000001', 'b'),
        ('11000003', 'b'),
        )


integrity_check = False

from basf2 import *
import interactive

from ROOT import Belle2
import numpy as np
import sys
# from testpkg.bitstring import BitArray
from bitstring import BitArray
import pickle
import re

[steering, srootFile] = sys.argv[:2]
if len(sys.argv) >= 3:
    pickleSigFile = sys.argv[2]
    isPickleFile = pickleSigFile[pickleSigFile.rfind('.'):] == '.p'
else:
    pickleSigFile = None
    isPickleFile = False

pickleIt = len(sys.argv) == 2 or isPickleFile


def finesse(x): return ord(x) - ord('a')


def copper(x): return int(x, 16)


hslb = [(copper(ele[0]), finesse(ele[1])) for ele in hslb]


def join(ary):
    return BitArray([]).join([BitArray(uint=i, length=32) for i in ary])


data = []
meta = []


class MinModule(Module):

    """
    Example module to drop into ipython and create some objects to look at.
    If you just want to start IPython and create PyStoreArray etc.
    interactively in your own steering file, the 'Interactive' module
    might be of interest.
    """

    def event(self):
        """
        reimplement Module::event()
        """
        self.return_value(0)
        # function namespace caching
        frombuffer = np.frombuffer
        GetNodeID = Belle2.RawTRG.GetNodeID
        GetDetectorNwords = Belle2.RawTRG.GetDetectorNwords
        GetDetectorBuffer = Belle2.RawTRG.GetDetectorBuffer
        trgary = Belle2.PyStoreArray("RawTRGs")

        evtmeta = Belle2.PyStoreObj("EventMetaData")
        trgs = []

        # integrity check
        if integrity_check:
            print(len(trgary), 'copper(s)')
            for evt in trgary:
                for entry in range(evt.GetNumEntries()):  # flattened. Usually only 1 entry
                    print('{:0x}'.format(GetNodeID(evt, entry)))
                    entrylist = []
                    for bid in range(4):
                        if (GetNodeID(evt, entry), bid) in hslb:
                            count = GetDetectorNwords(evt, entry, bid)
                            print(bid, "exist, ", count, "words.")
            return
        dataList = []
        for evt in trgary:
            # assuming smaller Copper ID comes first
            for entry in range(evt.GetNumEntries()):  # flattened. Usually only 1 entry
                # print('{:0x}'.format(GetNodeID(evt, entry)))
                entrylist = []
                for bid in range(4):
                    if (GetNodeID(evt, entry), bid) not in hslb:
                        continue
                    count = GetDetectorNwords(evt, entry, bid)
                    if count == 0:
                        # continue
                        pass
                    bf = GetDetectorBuffer(evt, entry, bid)
                    ary = frombuffer(bf, np.uintc, count)
                    # obsolete: skipping dummy buffers
                    # if '{:0x}'.format(ary[0])[:4] == 'dddd':
                    #     self.return_value(1)
                    # else:
                    #     return
                    self.return_value(1)
                    dataList.append(join(ary))

        event = evtmeta.getEvent()
        run = evtmeta.getRun()
        subrun = evtmeta.getSubrun()
        meta.append((event, run, subrun))
        data.append(dataList)


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Create main path
main = create_path()

# input
if srootFile[-5:] == 'sroot':
    root_input = register_module('SeqRootInput')
else:
    root_input = register_module('RootInput')
root_input.param('inputFileName', srootFile)

prog = register_module('Progress')

# Add modules to main path
main.add_module(root_input)
main.add_module(prog)

readout = MinModule()
main.add_module(readout)

emptypath = create_path()
readout.if_false(emptypath)

# check signal file before processing
if not pickleIt:
    import b2vcd_48
    vcdFile = sys.argv[3] if len(sys.argv) >= 4 else re.sub(r'.+/', '', re.sub(r'sroot', 'vcd', srootFile))
    with open(pickleSigFile) as fin:
        evtsize = [int(width) for width in fin.readline().split()]
        B2INFO('Interpreting B2L data format with dimension ' + str(evtsize))
        atlas = b2vcd_48.makeAtlas(fin.read(), evtsize)

# Process all events
process(main)

if pickleIt:
    pica = pickleSigFile if isPickleFile else re.sub(r'.+/', 'ana/', re.sub(r'sroot', 'p', srootFile))
    wfp = open(pica, 'wb')
    pickle.dump(data, wfp, protocol=2)
    pickle.dump(meta, wfp, protocol=2)
    wfp.close()

print(statistics)

if pickleIt:
    B2INFO('Output pickle file ' + pica + ' saved.')
else:
    b2vcd_48.writeVCD(meta, data, atlas, vcdFile, evtsize)
