#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from modularAnalysis import *

set_log_level(LogLevel.ERROR)

outTuple = str(sys.argv[1])

main = create_path()

inputMdstList('default', 'mdst_000001_prod00000870_task00000001.root', path=main)

fillParticleList('gamma:all', '', path=main)
fillParticleList('pi+:good', 'chiProb > 0.001 and piid > 0.5', path=main)

reconstructDecay('K_S0 -> pi+:good pi-:good', '0.480<=M<=0.516', 1, path=main)
# reconstructDecay('K_S0 -> pi+:good pi-:good', '0.480<=M<=0.5', 1, path=main)
reconstructDecay('pi0  -> gamma:all gamma:all', '0.115<=M<=0.152', 1, path=main)
reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2', path=main)

matchMCTruth('B0', path=main)

buildRestOfEvent('B0', path=main)
# appendROEMask('B0', 'simple', 'abs(d0) < 0.1', 'goodGamma == 1')

cleanMask = ('cleanMask', 'p > 0.1', 'p > 0.08 and clusterE9E25 > 0.9 and abs(clusterTiming) < 10', [0, 0, 1, 0, 0, 0])
defaultMask = ('defaultMask', '', '', [0, 0, 1, 0, 0, 0])
appendROEMasks('B0', [cleanMask, defaultMask])
# appendROEMasks('B0', [cleanMask])

buildContinuumSuppression('B0', 'cleanMask', path=main)


# # Define the variables for training.
# trainVars = [
#     'R2',
#     'thrustBm',
#     'thrustOm',
#     'cosTBTO',
#     'cosTBz',
#     'KSFWVariables(et)',
#     'KSFWVariables(mm2)',
#     'KSFWVariables(hso00)',
#     'KSFWVariables(hso02)',
#     'KSFWVariables(hso04)',
#     'KSFWVariables(hso10)',
#     'KSFWVariables(hso12)',
#     'KSFWVariables(hso14)',
#     'KSFWVariables(hso20)',
#     'KSFWVariables(hso22)',
#     'KSFWVariables(hso24)',
#     'KSFWVariables(hoo0)',
#     'KSFWVariables(hoo1)',
#     'KSFWVariables(hoo2)',
#     'KSFWVariables(hoo3)',
#     'KSFWVariables(hoo4)',
#     'CleoCone(1)',
#     'CleoCone(2)',
#     'CleoCone(3)',
#     'CleoCone(4)',
#     'CleoCone(5)',
#     'CleoCone(6)',
#     'CleoCone(7)',
#     'CleoCone(8)',
#     'CleoCone(9)'
# ]
#
# # Save target variable necessary for training.
# targetVar = ['isNotContinuumEvent']
#
# # Create output file.
# variablesToNTuple('B0', trainVars + targetVar, treename='tree', filename=outTuple, path=main)

process(main, 100)
print(statistics)
