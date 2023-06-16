#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ------------------------------------------------------------------------------------
# BG simulation using SAD input files
#
# make a link to the SAD files before running it (at KEKCC):
#  (ln -s /home/belle/staric/public/basf2/SADfiles input - sorry this files are outdated!)
#
# usage:
#    basf2 generateSADBg.py bgType accRing equivTime_us num [sampleType phase outdir]
# arguments:
#    bgType         Coulomb, Coulomb_base, Coulomb_dynamic, Touschek, Brems, Brems_base, Brems_dynamic
#    accRing        LER or HER
#    equivTime_us   equivalent SuperKEKB running time in micro-seconds
#    num            output file number
#    sampleType     one of: study, usual, PXD, ECL (D = usual)
#    phase          2, 31 (= early phase 3, ie pre-LS1), 32 (= pre-LS2) or 3 (D = 3)
#    outdir         output directory path (D = output)
# -------------------------------------------------------------------------------------

import basf2 as b2
import sys
import os
from background import add_output

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 5:
    bgType = argvs[1]     # Coulomb, Coulomb_base, Coulomb_dynamic, Touschek, Brems, Brems_base, Brems_dynamic
    accRing = argvs[2]    # LER or HER
    equivTime = argvs[3]  # equivalent SuperKEKB running time in micro-seconds
    num = argvs[4]        # output file number
    sampleType = 'usual'  # study, usual, PXD, ECL
    phase = 3             # phase number
    outdir = 'output'     # output directory path
elif argc == 6:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = 3
    outdir = 'output'
elif argc == 7:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = int(argvs[6])
    outdir = 'output'
elif argc == 8:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = int(argvs[6])
    outdir = argvs[7]
else:
    print('Usage:')
    print('  basf2', argvs[0], 'bgType accRing equivTime_us num [sampleType phase outdir]')
    print('Arguments:')
    print('  bgType         Coulomb, Coulomb_base, Coulomb_dynamic, Touschek, Brems, Brems_base, Brems_dynamic')
    print('  accRing        LER or HER')
    print('  equivTime_us   equivalent SuperKEKB running time in micro-seconds')
    print('  num            output file number')
    print('  sampleType     one of: study, usual, PXD, ECL (D = usual)')
    print('  phase          2, 31 (= early phase 3, ie pre-LS1), 32 (= pre-LS2) or 3 (D = 3)')
    print('  outdir         output directory path (D = output)')
    sys.exit()

# set sub-directories of SAD files

if phase == 3:
    subdir = 'phase3/'
elif phase == 31:
    subdir = 'phase3-early/'
elif phase == 32:
    subdir = 'phase3-preLS2/'
elif phase == 2:
    subdir = 'phase2/'
else:
    print('phase ', phase, 'not supported')
    sys.exit()

# set parameters

bgType = bgType + '_' + accRing
sadFile = 'input/' + subdir + bgType + '.root'
realTime = float(equivTime) * 1000
fname = bgType + '_' + sampleType + '-phase' + str(phase) + '-' + num
outputFile = outdir + '/' + fname + '.root'

# check for the existence of a SAD file

if not os.path.exists(sadFile):
    b2.B2ERROR('SAD file ' + sadFile + ' not found')
    sys.exit()

# make output directory if it doesn't exist

if not os.path.exists(outdir):
    os.makedirs(outdir)

# log message
b2.B2RESULT('SAD file (input): ' + sadFile)
b2.B2RESULT('Output file: ' + outputFile)
b2.B2RESULT('Corresponds to ' + equivTime + ' us of running phase ' + str(phase))

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.RESULT)

# Create path
main = b2.create_path()

# Event info setter
# Set some large number of events - processing will be stopped by BG generator
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000000])
main.add_module(eventinfosetter)

# Gearbox
gearbox = b2.register_module('Gearbox')
if phase == 2:
    gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
elif phase == 31:
    gearbox.param('fileName', 'geometry/Belle2_earlyPhase3.xml')
elif phase == 32:
    gearbox.param('fileName', 'geometry/Belle2_preLS2.xml')
if sampleType == 'study':
    gearbox.param('override', [
        ("/DetectorComponent[@name='PXD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='PXD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='SVD']//ActiveChips", 'true', ''),
        ("/DetectorComponent[@name='SVD']//SeeNeutrons", 'true', ''),
        ("/DetectorComponent[@name='TOP']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ARICH']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='ECL']//BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='KLM']//BKLM/BeamBackgroundStudy", '1', ''),
        ("/DetectorComponent[@name='KLM']//EKLM/BeamBackgroundStudy", '1', ''),
    ])
main.add_module(gearbox)

# BG generator
generator = b2.register_module('BeamBkgGenerator')
generator.param('fileName', sadFile)
generator.param('ringName', accRing)
generator.param('realTime', realTime)
main.add_module(generator)

# Geant geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
# add beast detectors for early phase3
if sampleType == 'study' and (phase == 31 or phase == 32):
    geometry.param('additionalComponents', ["BEAMABORT", "MICROTPC", "CLAWS", "HE3TUBE"])
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
if phase == 31 or phase == 32:
    phase = 3
add_output(main, bgType, realTime, sampleType, phase, fileName=outputFile)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
