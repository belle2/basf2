##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2

import sys

if len(sys.argv) != 3:
    sys.exit("Expected two arguments (input filename + output filename). Exit.")
filenameIn = sys.argv[1]
filenameOut = sys.argv[2]

b2.set_log_level(b2.LogLevel.WARNING)

main = b2.create_path()

main.add_module('RootInput', inputFileName=filenameIn)
main.add_module('Gearbox')
main.add_module('Geometry')

# add CDC trigger (only TSF)
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v2.2.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v2.2.coe"))

# add ECL trigger
main.add_module('TRGECLFAM',
                TCWaveform=0,
                FAMFitMethod=1,
                TCThreshold=100,
                BeamBkgTag=0,
                ShapingFunction=1)
main.add_module('TRGECL',
                Clustering=0,
                EventTiming=1,
                Bhabha=0,
                EventSelect=0,
                TimeWindow=375)

# add decision module and skip events that are not triggered
not_triggered = b2.create_path()

trigger = main.add_module('TRGGDLCosmicRun', BackToBack=True)
trigger.if_false(not_triggered)

main.add_module('RootOutput', outputFileName=filenameOut)
