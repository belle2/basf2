#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# steering file - 2017 Belle II Collaboration
######################################################

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


def join(ary):
    return BitArray([]).join([BitArray(uint=i, length=32) for i in ary])


def printBin(evt, wordwidth=8, linewidth=8, paraheight=4):
    words = [evt[word:word + wordwidth].bin for word in range(0, len(evt), wordwidth)]
    lines = ([' '.join(words[n:n + linewidth]) for n in range(0, len(words), linewidth)])
    paras = (['\n'.join(lines[n:n + paraheight]) for n in range(0, len(lines), paraheight)])
    print('\n\n'.join(paras))


def printHex(evt, wordwidth=32, linewidth=8, paraheight=4):
    words = [evt[word:word + wordwidth].hex for word in range(0, len(evt), wordwidth)]
    lines = ([' '.join(words[n:n + linewidth]) for n in range(0, len(words), linewidth)])
    paras = (['\n'.join(lines[n:n + paraheight]) for n in range(0, len(lines), paraheight)])
    print('\n\n'.join(paras))


data = []
if pickleIt:
    pica = pickleSigFile if isPickleFile else re.sub(r'.+/', '', re.sub(r'sroot', 'p', srootFile))
    wfp = open(pica, 'wb')
    pickle.dump(data, wfp, protocol=2)
    wfp.close()


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
        trgary = Belle2.PyStoreArray("RawTRGs")
        trgs = []
        for evt in trgary:
            for entry in range(evt.GetNumEntries()):  # flattened. Usually only 1 entry
                entrylist = [BitArray([])] * 4
                for bid in range(4):
                    # if bid != 1: continue # TSF2 uses Finesse B
                    # if bid != 0: continue # 2D uses Finesse A
                    count = evt.GetDetectorNwords(entry, bid)
                    # print(bid, count)
                    if count == 0:
                        continue
                    bf = evt.GetDetectorBuffer(entry, bid)
                    ary = np.frombuffer(bf, np.uintc, count)
                    # print( ['{:x}'.format(nu) for nu in ary[:10]] )
                    entrylist[bid] = join(ary)
                trgs.append(entrylist)
        # B2INFO(80 * '=')
        # B2INFO('You are now in basf2 session')
        # B2INFO('TRG B2L data are loaded now. Try these:')
        # B2INFO('>>> printHex(trgs[0])')
        # B2INFO('>>> printBin(trgs[1], wordwidth=4, linewidth=10, paraheight=5)')
        # B2INFO(80 * '=')
        # interactive.embed()

        if not pickleIt:
            global data
        else:
            rfp = open(pica, 'rb')
            data = pickle.load(rfp)
            rfp.close()

        data.append(trgs)

        if pickleIt:
            wfp = open(pica, 'wb')
            pickle.dump(data, wfp, protocol=2)
            wfp.close()


# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Create main path
main = create_path()

# input
input = register_module('SeqRootInput')
input.param('inputFileName', srootFile)

# readout
# readout = register_module('ReadOut')

# dump = register_module('SeqRootOutput')
# dump = register_module('RootOutput')
# dump.param('outputFileName', 'readout.root')

prog = register_module('Progress')


# Add modules to main path
main.add_module(input)
# main.add_module(readout)
# main.add_module(dump)
main.add_module(prog)
main.add_module(MinModule())

# check signal file before processing
if not pickleIt:
    import b2vcd
    vcdFile = sys.argv[3] if len(sys.argv) >= 4 else re.sub(r'.+/', '', re.sub(r'sroot', 'vcd', srootFile))
    with open(pickleSigFile) as fin:
        evtsize = [int(width) for width in fin.readline().split()]
        B2INFO('Interpreting B2L data format with dimension ' + str(evtsize))
        atlas = b2vcd.makeAtlas(fin.read(), evtsize)

# Process all events
process(main)

if pickleIt:
    B2INFO('Output pickle file ' + pica + ' saved.')
else:
    b2vcd.writeVCD(data, atlas, vcdFile, evtsize)
