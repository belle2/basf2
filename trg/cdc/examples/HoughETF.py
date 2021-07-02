#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        CDCTrg Hough ETF  module
#
#    usage : %> basf2 HoughETF.py -i [input root file name]
#
# -----------------------------------------------------------------------------------
import basf2 as b2
from ROOT import Belle2


input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = b2.register_module('SeqRootInput')
else:
    root_input = b2.register_module('RootInput')

main = b2.create_path()
main.add_module(root_input)

# Set Database
b2.conditions.override_globaltags()
b2.use_database_chain()
b2.use_central_database("data_reprocessing_prompt")
b2.use_central_database("online")

# b2.set_log_level(b2.LogLevel.DEBUG)
b2.set_log_level(b2.LogLevel.INFO)

# cdc unpacker
cdc_unpacker = b2.register_module('CDCUnpacker')
cdc_unpacker.param('enableStoreCDCRawHit', True)
main.add_module(cdc_unpacker)
main.add_module('Gearbox')
main.add_module('Geometry', components=['CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
# cdc digitizer
cdcdigitizer = b2.register_module('CDCDigitizer')
param_cdcdigi = {'Fraction': 1,
                 'Resolution1': 0.,
                 'Resolution2': 0.}
cdcdigitizer.param(param_cdcdigi)
cdcdigitizer.param('AddInWirePropagationDelay', True)
cdcdigitizer.param('AddTimeOfFlight', True)
cdcdigitizer.param('UseSimpleDigitization', True)
cdcdigitizer.param('UseDB4EDepToADC', False)
cdcdigitizer.param('UseDB4FEE', False)
cdcdigitizer.param('AddXTalk', False)
main.add_module(cdcdigitizer)

# cdc tsf
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"),
                TSHitCollectionName='CDCTriggerSegmentHits')

useETF = True

if useETF:
    # cdc trigger etf (exp14 runXXXX-)
    main.add_module('CDCTriggerHoughETF',
                    t0CalcMethod=2,
                    useHighPassTimingList=False,
                    usePriorityTiming=False,
                    storeTracks=True,
                    hitCollectionName="CDCTriggerSegmentHits",
                    outputCollectionName="CDCTriggerETFTracks")
else:
    # cdc trigger fastest priority timing (GRL timing)
    main.add_module('CDCTriggerHoughETF',
                    hitCollectionName="CDCTriggerSegmentHits")


main.add_module('Progress')

# main.add_module(output);
output_name = 'etfout.root'
main.add_module('RootOutput', outputFileName=output_name)

# Process all events
b2.process(main)
