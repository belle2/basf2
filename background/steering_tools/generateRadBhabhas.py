#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -------------------------------------------------------------------------------------
# BG simulation: Radiative Bhabha's
# usage:
#    basf2 generateRadBhabhas.py generator equivTime_us num [sampleType phase outdir]
# arguments:
#    generator      one of: bbbrem, bhwide, bhwide_largeangle
#    equivTime_us   equivalent SuperKEKB running time in micro-seconds
#    num            output file number
#    sampleType     one of: study, usual, PXD, ECL (D = usual)
#    phase          2, 31 (= early phase 3, ie Run 1), 32 (= Run 2) or 3 (D = 3)
#    outdir         output directory path (D = output)
# -------------------------------------------------------------------------------------

import basf2 as b2
import sys
import os
from background import add_output

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 4:
    generator = argvs[1]  # bbbrem, bhwide, bhwide_largeangle
    equivTime = argvs[2]  # equivalent SuperKEKB running time in micro-seconds
    num = argvs[3]        # output file number
    sampleType = 'usual'  # study, usual, PXD, ECL
    phase = 3             # phase number
    outdir = 'output'     # output directory path
elif argc == 5:
    generator = argvs[1]
    equivTime = argvs[2]
    num = argvs[3]
    sampleType = argvs[4]
    phase = 3
    outdir = 'output'
elif argc == 6:
    generator = argvs[1]
    equivTime = argvs[2]
    num = argvs[3]
    sampleType = argvs[4]
    phase = int(argvs[5])
    outdir = 'output'
elif argc == 7:
    generator = argvs[1]
    equivTime = argvs[2]
    num = argvs[3]
    sampleType = argvs[4]
    phase = int(argvs[5])
    outdir = argvs[6]
else:
    print('Usage:')
    print('  basf2', argvs[0], 'generator equivTime_us num [sampleType phase outdir]')
    print('Arguments:')
    print('  generator     one of: bbbrem, bhwide, bhwide_largeangle')
    print('  equivTime_us  equivalent SuperKEKB running time in micro-seconds')
    print('  num           output file number')
    print('  sampleType    one of: study, usual, PXD, ECL (D = usual)')
    print('  phase         2, 31 (= early phase 3, ie Run 1), 32 (= Run 2) or 3 (D = 3)')
    print('  outdir        output directory path (D = output)')
    sys.exit()

# set parameters

if generator == 'bbbrem':
    bgType = 'RBB'
    crossect = 524e6 * 2  # nb (factor of two: gamma emission by e- and by e+)
elif generator == 'bhwide':
    bgType = 'BHWide'
    crossect = 123e3  # nb
elif generator == 'bhwide_largeangle':
    bgType = 'BHWideLargeAngle'
    crossect = 123e3  # nb
else:
    print('unknown generator: ', generator)
    sys.exit()

if phase == 3:
    lumi = 600  # /nb/s (1/nb/s = 1e33/cm^2/s)
elif phase == 31:
    lumi = 30  # /nb/s
elif phase == 32:
    lumi = 280  # /nb/s
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

# make output directory if it doesn't exist

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
kill = b2.create_path()


def add_cut(name, minParticles, maxParticles, minTheta, maxTheta=None):
    """Add a generator level cut and kill the event if the cut is not passed.  In
    this case the cut is on the min/max charged particles which have a
    center-of-mass theta angle between minTheta and maxTheta. If maxTheta is not
    given assume it to be 180-minTheta for a symmetric window"""

    # if only one angle make it symmetric
    if maxTheta is None:
        maxTheta = 180 - minTheta
    selection = main.add_module('GeneratorPreselection')
    selection.param('applyInCMS', True)
    selection.param('nChargedMin', minParticles)
    selection.param('nChargedMax', maxParticles)
    selection.param('MinChargedTheta', minTheta)
    selection.param('MaxChargedTheta', maxTheta)
    selection.param('MinChargedP', 0.)
    selection.param('MinChargedPt', 0.)
    selection.if_value("!=11", kill)
    selection.set_name("generator cut: " + name)


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
elif phase == 32:
    gearbox.param('fileName', 'geometry/Belle2_Run2.xml')
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

# Generator
if generator == "bbbrem":
    main.add_module("BBBremInput", MinPhotonEnergyFraction=0.000001, Unweighted=True,
                    MaxWeight=1.57001e+07)
    # at least one track below 0.5 degree means maximum one particle in 0.5-179.5
    add_cut("at least one track below 0.5 degree", 0, 1, 0.5)
elif generator == "bhwide":
    main.add_module("BHWideInput", ScatteringAngleRangeElectron=[0.5, 179.5],
                    ScatteringAngleRangePositron=[0.5, 179.5])
    add_cut("both tracks at least 0.5 degree", 2, 2, 0.5)
    # but if one is above 1 and the other above 10 degree so we in 1-170 and
    # 10-179
    add_cut("max one track in 1-170", 0, 1, 1, 170)
    add_cut("max one track in 10-179", 0, 1, 10, 179)
elif generator == "bhwide_largeangle":
    main.add_module("BHWideInput", ScatteringAngleRangeElectron=[0.5, 179.5],
                    ScatteringAngleRangePositron=[0.5, 179.5])
    add_cut("both tracks at least 1 degree", 2, 2, 1)
    add_cut("at least one 10 degree", 1, 2, 10)
else:
    print("unknown generation setting: {}".format(generator))

# Geant geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
addComp = ["MagneticField3dQuadBeamline"]
# add beast detectors
if sampleType == 'study' and (phase == 31 or phase == 32):
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
if phase == 31 or phase == 32:
    phase = 3
add_output(main, bgType, realTime, sampleType, phase, fileName=outputFile)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
