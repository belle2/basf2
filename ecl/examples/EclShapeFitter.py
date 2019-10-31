#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------
# Performs shape fit and compares emulator fit results with fit results
# received from ShaperDSP module.
# --------------------------------------------------------------------------

from basf2 import *
from ROOT import Belle2, gSystem, gInterpreter
from glob import glob

from ROOT import TFile, TTree
from array import array

env = Belle2.Environment.Instance()


################################################
# PARAMETERS
################################################

# Name of input files (accepts glob expressions)
FILE_LIST = []
FILE_LIST = sorted(glob('/group/belle2/dataprod/Data/Raw/e0008/r03480/sub00/*.root'))

# Override if "-i file.root" argument was sent to basf2.
input_arg = env.getInputFilesOverride()
if len(input_arg) > 0:
    FILE_LIST = [str(x) for x in input_arg]
print(FILE_LIST)

# Output file name
OUTPUT = "out.root"
# Override output if "-o file.root" argument was sent to basf2.
output_arg = env.getOutputFileOverride()
if len(output_arg) > 0:
    OUTPUT = output_arg

VERBOSE = False

################################################
# BASF2 PATH GENERATION
################################################


'''
Module that prints ShaperDSP emulator discrepancies
for ECL data.

Uses ECLDigits, ECLDsps, ECLTrigs dataobjects
'''


class ShapeFitterModule(Module):
    def initialize(self):
        self.evtn = 0
        self.digits = Belle2.PyStoreArray('ECLDigits')

    def event(self):
        '''
        Check for discrepancy between real ShaperDSP data and shapeFitter function
        from ecl/utility/src/ECLDspUtilities.cc
        '''

        for digit in self.digits:
            waveform = digit.getRelated('ECLDsps')
            if not waveform:
                continue

            trig = digit.getRelated('ECLTrigs')
            if not trig:
                continue

            trigger_time = int(trig.getTimeTrig())

            # Waveform data
            adc = waveform.getDspA()

            cid = digit.getCellId()
            amp = digit.getAmp()
            time = digit.getTimeFit()
            qual = digit.getQuality()

            # == Call emulator
            result = Belle2.ECL.ECLDspUtilities.shapeFitter(cid, adc, trigger_time)

            cid2 = result.getCellId()
            amp2 = result.getAmp()
            time2 = result.getTimeFit()
            qual2 = result.getQuality()

            if amp != amp2 or time != time2 or qual != qual2:
                print()
                print('RealData: %4d %6d %6d %6d' % (cid, amp, time, qual))
                print('Emulator: %4d %6d %6d %6d' % (cid2, amp2, time2, qual2))
                if VERBOSE:
                    print('Event : %d Trigger time: %d' % (self.evtn, trigger_time))
                    print('CellID: %d AmpData: %d TimeData: %d QualityData: %d' % (cid, amp, time, qual))
                    print(' '.join([str(x) for x in adc]), end='')
                    print(' ')
                    self.evtn += 1


set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# (Seq)Root input
if FILE_LIST[0].endswith('sroot'):
    main.add_module('SeqRootInput', inputFileName="", inputFileNames=FILE_LIST)
else:
    main.add_module('RootInput', inputFileName="", inputFileNames=FILE_LIST)

# if FILE_LIST[0].endswith('sroot') or 'Raw' in FILE_LIST[0]:
main.add_module('ECLUnpacker', storeTrigTime=True)

# Do shape fitting of ECLDsps
main.add_module(ShapeFitterModule())

main.add_module('Progress')

reset_database()
use_database_chain()
use_central_database('data_reprocessing_prompt', LogLevel.WARNING)
use_central_database('online', LogLevel.WARNING)
use_local_database("localdb/database.txt")

# For exp9 data
conditions.override_globaltags()

# Process events
process(main)

print(statistics)
