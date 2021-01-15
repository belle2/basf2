#!/usr/bin/env python
# -*- coding: utf-8 -*-

# -------------------------------------------------------------------------------------
# BG simulation: two-photon (QED)
# usage:
#    basf2 generateTwoPhoton.py equivTime_us num [sampleType phase outdir]
# arguments:
#    equivTime_us   equivalent SuperKEKB running time in micro-seconds
#    num            output file number
#    sampleType     one of: study, usual, PXD, ECL
#    phase          2, 31 (= early phase 3) or 3
#    outdir         output directory path
# -------------------------------------------------------------------------------------

import basf2 as b2
import sys
import os
from background import add_output

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 3:
    equivTime = argvs[1]  # equivalent SuperKEKB running time in micro-seconds
    num = argvs[2]        # output file number
    sampleType = 'usual'  # study, usual, PXD, ECL
    phase = 3             # phase number
    outdir = 'output'     # output directory path
elif argc == 4:
    equivTime = argvs[1]
    num = argvs[2]
    sampleType = argvs[3]
    phase = 3
    outdir = 'output'
elif argc == 5:
    equivTime = argvs[1]
    num = argvs[2]
    sampleType = argvs[3]
    phase = int(argvs[4])
    outdir = 'output'
elif argc == 6:
    equivTime = argvs[1]
    num = argvs[2]
    sampleType = argvs[3]
    phase = int(argvs[4])
    outdir = argvs[5]
else:
    print('usage:')
    print('  basf2', argvs[0], 'equivTime_us num [sampleType phase outdir]')
    print('arguments:')
    print('  equivTime_us  equivalent SuperKEKB running time in micro-seconds')
    print('  num           output file number')
    print('  sampleType    one of: study, usual, PXD, ECL')
    print('  phase         2, 31 (=early phase 3) or 3')
    print('  outdir        output directory path')
    sys.exit()

# set parameters

bgType = 'twoPhoton'
crossect = 7.28e6  # nb

if phase == 3:
    lumi = 800  # /nb/s
elif phase == 31:
    lumi = 250  # /nb/s
elif phase == 2:
    lumi = 20   # /nb/s
else:
    print('phase ', phase, 'not supported')
    sys.exit()

realTime = float(equivTime) * 1000  # ns
numEvents = int(crossect * lumi * realTime * 1e-9)
fname = bgType + '_' + sampleType + '-phase' + str(phase) + '-' + num
outputFile = outdir + '/' + fname + '.root'

if numEvents == 0:
    b2.B2ERROR('number of events is 0 -> increase equivTime_us')
    sys.exit()

# make output directory if doesn't exists

if not os.path.exists(outdir):
    os.makedirs(outdir)

# log messages

b2.B2RESULT('Events to be generated: ' + str(numEvents) + ' - corresponds to ' + equivTime +
            ' us of running at ' + str(lumi) + ' /nb/s (phase ' + str(phase) + ')')
b2.B2RESULT('Output file: ' + outputFile)

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.RESULT)

# Create path
main = b2.create_path()

# Event info setter
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)

# Gearbox
gearbox = b2.register_module('Gearbox')
if phase == 2:
    gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
elif phase == 31:
    gearbox.param('fileName', 'geometry/Belle2_earlyPhase3.xml')
if sampleType == 'study':
    gearbox.param('override', [
        ("/DetectorComponent[@name='PXD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='PXD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='SVD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='SVD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='TOP']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ARICH']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ECL']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='KLM']//BeamBackgroundStudy", '1', ''),
    ])
main.add_module(gearbox)

# Generator (settings from Nakayama-san 15th campaign)
aafh = b2.register_module('AafhInput')
aafh.param({
    'mode': 5,
    'rejection': 2,
    'suppressionLimits': [1e100] * 4,
    'minMass': 0.001,
    'maxFinalWeight': 2.5,
    'maxSubgeneratorWeight': 1.0,
    'subgeneratorWeights': [1.000e+00, 2.216e+01, 3.301e+03, 6.606e+03, 1.000e+00,
                            1.675e+00, 5.948e+00, 6.513e+00],
})
main.add_module(aafh)

# Geant geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
addComp = ["MagneticField3dQuadBeamline"]
# add beast detectors for early phase3
if phase == 31 and sampleType == 'study':
    addComp.extend(["BEAMABORT", "MICROTPC", "CLAWS", "HE3TUBE"])

geometry.param({"excludedComponents": ["MagneticField"],
                "additionalComponents": addComp})
main.add_module(geometry)

# Geant simulation
fullsim = b2.register_module('FullSim')
if sampleType == 'study':
    fullsim.param('PhysicsList', 'FTFP_BERT_HP')
    fullsim.param('UICommandsAtIdle', ['/process/inactivate nKiller'])
    fullsim.param('StoreAllSecondaries', True)
    fullsim.param('SecondariesEnergyCut', 0.000001)  # [MeV] need for CDC EB neutron flux
main.add_module(fullsim)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Output
if phase == 31:
    phase = 3
add_output(main, bgType, realTime, sampleType, phase, fileName=outputFile)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
