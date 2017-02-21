#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Prints a summary of raw data given in Interim FE format v2.1
# usage: basf2 checkInterimFEData.py -i <file_name.sroot> [-n <num_of_events_to_process>]
# ---------------------------------------------------------------------------------------

from basf2 import *
from ROOT import Belle2


class InspectRawTOP(Module):
    ''' Print summary of raw data given in Interim FE format '''

    def swap32(self, x):
        '''' Swap bytes '''

        return (((x << 24) & 0xFF000000) |
                ((x << 8) & 0x00FF0000) |
                ((x >> 8) & 0x0000FF00) |
                ((x >> 24) & 0x000000FF))

    def event(self):
        ''' Event processor '''

        rawTOPs = Belle2.PyStoreArray('RawTOPs')
        if rawTOPs.getEntries() == 0:
            print('RawTOP is empty - maybe component ID was not set properly at DAQ')
        indx = 0
        for raw in rawTOPs:
            expno = raw.GetExpNo(0)
            runno = raw.GetRunNo(0)
            subno = raw.GetSubRunNo(0)
            eveno = raw.GetEveNo(0)
            copper = raw.GetNodeID(0) & 0x00FFFFFF
            print('RawTOP[' + str(indx) + ']:',
                  'COPPER=' + str(copper),
                  'EXP=' + str(expno),
                  'RUN=' + str(runno),
                  'SUBRUN=' + str(subno),
                  'EVENT=' + str(eveno))
            indx = indx + 1
            for finesse in range(4):
                size = raw.GetDetectorNwords(0, finesse)
                if size == 0:
                    print(' HLSB', str(finesse),
                          ': size =', str(size), 'words')
                    continue
                data = raw.GetDetectorBuffer(0, finesse)
                scrod = self.swap32(data[0]) & 0x0FFF
                numFE = 0
                numWF = 0
                numPoz = 0
                numNeg = 0
                for i in range(size):
                    word = self.swap32(data[i])
                    k = 0
                    if word == 0xaaaa0100:
                        numFE = numFE + 1
                        k = i + 4
                    elif word == 0xaaaa0103:
                        numFE = numFE + 1
                        numWF = numWF + 1
                        k = i + 4
                    if k > 0 and k < size - 6:
                        if self.swap32(data[k]) != 0xd8f1ffff:
                            numPoz = numPoz + 1
                        if self.swap32(data[k + 6]) != 0x270fffff:
                            numNeg = numNeg + 1
                print(' HLSB', str(finesse),
                      ': size =', str(size), 'words,',
                      'SCROD', str(scrod) + ',',
                      'FE headers=' + str(numFE),
                      'signalsPoz=' + str(numPoz),
                      'signalsNeg=' + str(numNeg),
                      'Waveforms=' + str(numWF))
        print()


# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
converter = register_module('Convert2RawDet')
main.add_module(converter)

# event info printer
main.add_module('EventInfoPrinter')

# summary print-out
main.add_module(InspectRawTOP())

# Process events
process(main)
