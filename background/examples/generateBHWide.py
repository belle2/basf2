#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys
from background import add_output

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 3:
    equivTime = argvs[1]  # equivalent SuperKEKB running time in micro-seconds
    num = argvs[2]        # output file number
    sampleType = 'usual'  # study, usual, PXD, ECL
    phase = 3             # 1, 2 or 3
elif argc == 4:
    equivTime = argvs[1]
    num = argvs[2]
    sampleType = argvs[3]
    phase = 3
elif argc == 5:
    equivTime = argvs[1]
    num = argvs[2]
    sampleType = argvs[3]
    phase = argvs[4]
else:
    print('usage:')
    print('basf2', argvs[0], '(equivTime_us num [(study,usual,ECL,PXD) phase]')
    sys.exit()

bgType = 'BHWide'
realTime = float(equivTime) * 1000  # [ns]
numEvents = int(984 / 10.0e3 * realTime)  # conversion (Nakayama-san)
fname = bgType + '_' + sampleType + '-' + num
outputFile = 'output/' + fname + '.root'

print('Events to be generated:', numEvents)

# Suppress messages and warnings during processing:
set_log_level(LogLevel.RESULT)

# Create path
main = create_path()

# Event info setter
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)

# Gearbox
gearbox = register_module('Gearbox')
if sampleType == 'study':
    gearbox.param('override', [
        ("/DetectorComponent[@name='PXD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='PXD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='SVD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='SVD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='TOP']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ARICH']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ECL']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='BKLM']//BeamBackgroundStudy", '1', ''),
    ])
main.add_module(gearbox)

# generator
bhwide = register_module('BHWideInput')
bhwide.param('ScatteringAngleRangePositron', [0.5, 179.5])
bhwide.param('ScatteringAngleRangeElectron', [0.5, 179.5])
main.add_module(bhwide)

# Geant geometry
geometry = register_module('Geometry')
geometry.param({"excludedComponents": ["MagneticField"],
                "additionalComponents": ["MagneticField3dQuadBeamline"], })
main.add_module(geometry)

# Geant simulation
fullsim = register_module('FullSim')
if sampleType == 'study':
    fullsim.param('PhysicsList', 'FTFP_BERT_HP')
    fullsim.param('UICommands', ['/process/inactivate nKiller'])
    fullsim.param('StoreAllSecondaries', True)
    fullsim.param('SecondariesEnergyCut', 0.000001)  # [MeV] need for CDC EB neutron flux
main.add_module(fullsim)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Output
add_output(main, bgType, realTime, sampleType, phase, fileName=outputFile)

# Process events
process(main)

# Print call statistics
print(statistics)
