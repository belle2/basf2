#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -------------------------------------------------------------------------
# BG simulation using SAD input files
#
# make a link to the SAD files before running it (at KEKCC):
#    ln -s /home/belle/staric/public/basf2/SADfiles/phase3-15th input
# -------------------------------------------------------------------------

from basf2 import *
import sys
from background import add_output

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 5:
    bgType = argvs[1]     # Coulomb, Touschek
    accRing = argvs[2]    # LER or HER
    equivTime = argvs[3]  # equivalent SuperKEKB running time in micro-seconds
    num = argvs[4]        # output file number
    sampleType = 'usual'  # study, usual, PXD, ECL
    phase = 3             # 1, 2 or 3
elif argc == 6:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = 3
elif argc == 7:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = argvs[6]
else:
    print('usage:')
    print('basf2', argvs[0],
          '(Touschek,Coulomb) (HER,LER) equivTime_us num [(study,usual,ECL,PXD) phase]')
    sys.exit()

bgType = bgType + '_' + accRing
sadFile = 'input/' + bgType + '.root'
realTime = float(equivTime) * 1000
fname = bgType + '_' + sampleType + '-' + num
outputFile = 'output/' + fname + '.root'

# Suppress messages and warnings during processing:
set_log_level(LogLevel.RESULT)

# Create path
main = create_path()

# Event info setter
# Set some large number of events - processing will be stopped by BG generator
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000000])
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

# BG generator
generator = register_module('BeamBkgGenerator')
generator.param('fileName', sadFile)
generator.param('ringName', accRing)
generator.param('realTime', realTime)
main.add_module(generator)

# Geant geometry
geometry = register_module('Geometry')
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

add_output(main, bgType, realTime, sampleType, phase, fileName=outputFile)

# Process events
process(main)

# Print call statistics
print(statistics)
