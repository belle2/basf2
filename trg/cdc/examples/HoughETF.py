#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        CDCTrg Hough ETF  module
#
#    usage : %> basf2 HoughETF.py -i [input root file name]
#
# -----------------------------------------------------------------------------------
from basf2 import *
from ROOT import Belle2


input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = register_module('SeqRootInput')
else:
    root_input = register_module('RootInput')

main = create_path()
main.add_module(root_input)

set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# cdc unpacker
cdc_unpacker = register_module('CDCUnpacker')
cdc_unpacker.param('enableStoreCDCRawHit', True)
main.add_module(cdc_unpacker)
main.add_module('Gearbox')
main.add_module('Geometry', components=['CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
# cdc digitizer
cdcdigitizer = register_module('CDCDigitizer')
param_cdcdigi = {'Fraction': 1,
                 'Resolution1': 0.,
                 'Resolution2': 0.}
cdcdigitizer.param(param_cdcdigi)
cdcdigitizer.param('AddInWirePropagationDelay', True)
cdcdigitizer.param('AddTimeOfFlight', True)
cdcdigitizer.param('UseSimpleDigitization', True)
main.add_module(cdcdigitizer)

# cdc tsf
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"),
                TSHitCollectionName='CDCTriggerSegmentHits')

# cdc trigger hough etf
main.add_module('CDCTriggerHoughETF',
                storeTracks=True,
                hitCollectionName="CDCTriggerSegmentHits",
                outputCollectionName="CDCTriggerETFTracks")


# main.add_module(output);
output_name = 'etfout.root'
main.add_module('RootOutput', outputFileName=output_name)

# Process all events
process(main)
