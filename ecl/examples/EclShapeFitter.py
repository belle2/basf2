#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Performs shape fit and compares emulator fit results with fit
   results received from ShaperDSP module.
"""

import glob
import basf2 as b2
from ROOT import Belle2

env = Belle2.Environment.Instance()

# PARAMETERS

inputFile_list = []
outputFileName = 'out.root'
inputFile_list = sorted(glob.glob('/group/belle2/dataprod/Data/Raw/e0008/r03480/sub00/*.root'))

# Override if "-i file.root" argument was sent to basf2.
input_arg = env.getInputFilesOverride()
if len(input_arg) > 0:
    inputFile_list = [str(x) for x in input_arg]
print(inputFile_list)

# Override output if '-o file.root' argument was sent to basf2.
output_arg = env.getOutputFileOverride()
if len(output_arg) > 0:
    outputFileName = output_arg

verbose = False


class ShapeFitterModule(b2.Module):

    """Module that prints ShaperDSP emulator discrepancies
       for ECL data.

    Uses ECLDigits, ECLDsps, ECLTrigs dataobjects
    """

    def initialize(self):
        """Initialize
        """
        #: event number
        self.eventNumber = 0
        #: Store array of ECLDigits
        self.digits = Belle2.PyStoreArray('ECLDigits')

    def event(self):
        """Check for discrepancy between real ShaperDSP
           data and shapeFitter function from
           ecl/utility/src/ECLDspUtilities.cc .
        """

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

            amp2 = result.amp
            time2 = result.time
            qual2 = result.quality

            if amp != amp2 or time != time2 or qual != qual2:
                print()
                print('RealData: %4d %6d %6d %6d' % (cid, amp, time, qual))
                print('Emulator: %4d %6d %6d %6d' % (cid, amp2, time2, qual2))
                if verbose:
                    print('Event : %d Trigger time: %d' % (self.evtn, trigger_time))
                    print('CellID: %d AmpData: %d TimeData: %d QualityData: %d' % (cid, amp, time, qual))
                    print(' '.join([str(x) for x in adc]), end='')
                    print(' ')

                    self.eventNumber += 1


b2.set_log_level(b2.LogLevel.ERROR)

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Add '(Seq)RootInput' module
if inputFile_list[0].endswith('sroot'):
    mainPath.add_module('SeqRootInput',
                        inputFileName='',
                        inputFileNames=inputFile_list)
else:
    mainPath.add_module('RootInput',
                        inputFileName='',
                        inputFileNames=inputFile_list)

# if inputFile_list[0].endswith('sroot') or 'Raw' in inputFile_list[0]:
mainPath.add_module('ECLUnpacker', storeTrigTime=True)

# Do shape fitting of ECLDsps
mainPath.add_module(ShapeFitterModule())

b2.reset_database()
b2.use_database_chain()
b2.use_central_database('data_reprocessing_prompt', b2.LogLevel.WARNING)
b2.use_central_database('online', b2.LogLevel.WARNING)
b2.use_local_database('localdb/database.txt')

# For exp9 data
b2.conditions.override_globaltags()

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
