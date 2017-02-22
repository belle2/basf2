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
reconstructDecay('pi0  -> gamma:all gamma:all', '0.115<=M<=0.152', 1, path=main)
reconstructDecay('B0   -> K_S0 pi0', '5.2 < Mbc < 5.3 and -0.3 < deltaE < 0.2', path=main)

matchMCTruth('B0', path=main)

buildRestOfEvent('B0', path=main)

cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
# cleanMask = ('cleanMask', '', '')
appendROEMasks('B0', [cleanMask], path=main)

buildContinuumSuppression('B0', 'cleanMask', path=main)
# buildContinuumSuppression('B0', '', path=main)

# Define the variables for training.
trainVars = [
    'R2EventLevel',
    'R2',
    'thrustBm',
    'thrustOm',
    'cosTBTO',
    'cosTBz',
    'KSFWVariables(et)',
    'KSFWVariables(mm2)',
    'KSFWVariables(hso00)',
    'KSFWVariables(hso02)',
    'KSFWVariables(hso04)',
    'KSFWVariables(hso10)',
    'KSFWVariables(hso12)',
    'KSFWVariables(hso14)',
    'KSFWVariables(hso20)',
    'KSFWVariables(hso22)',
    'KSFWVariables(hso24)',
    'KSFWVariables(hoo0)',
    'KSFWVariables(hoo1)',
    'KSFWVariables(hoo2)',
    'KSFWVariables(hoo3)',
    'KSFWVariables(hoo4)',
    'CleoCone(1)',
    'CleoCone(2)',
    'CleoCone(3)',
    'CleoCone(4)',
    'CleoCone(5)',
    'CleoCone(6)',
    'CleoCone(7)',
    'CleoCone(8)',
    'CleoCone(9)',
    'CleoCone(1,ROE)',
    'CleoCone(2,ROE)',
    'CleoCone(3,ROE)',
    'CleoCone(4,ROE)',
    'CleoCone(5,ROE)',
    'CleoCone(6,ROE)',
    'CleoCone(7,ROE)',
    'CleoCone(8,ROE)',
    'CleoCone(9,ROE)'
]

# Save target variable necessary for training.
targetVar = ['isNotContinuumEvent']

# Create output file.
variablesToNTuple('B0', trainVars + targetVar, treename='tree', filename=outTuple, path=main)

# toolsB = ['ContinuumSuppression[KsfwFS1CcROE]', '^B0']
# toolsB += ['ContinuumSuppression', '^B0']
# ntupleFile('nTupleTools.root', path=main)
# ntupleTree('ntupB0', 'B0', toolsB, path=main)

process(main, 1000)
print(statistics)
