from basf2 import *
from ROOT import Belle2

import sys

if len(sys.argv) != 3:
    sys.exit("Expected two arguments (input filename + output filename). Exit.")
filenameIn = sys.argv[1]
filenameOut = sys.argv[2]

set_log_level(LogLevel.WARNING)

main = create_path()

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
not_triggered = create_path()

trigger = main.add_module('TRGGDLCosmicRun', BackToBack=True)
trigger.if_false(not_triggered)

main.add_module('RootOutput', outputFileName=filenameOut)
