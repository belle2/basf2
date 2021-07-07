#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
# uses phase2 SAD files from /group/belle2/BGcampaigns/SAD/forG4/
#
# usage:
#    basf2 generateSADBg.py bgType accRing equivTime_us num [sampleType phase outdir]
# arguments:
#    bgType         Coulomb, Touschek or Brems
#    accRing        LER or HER
#    equivTime_us   equivalent SuperKEKB running time in micro-seconds
#    num            output file number
#    sampleType     one of: study, usual, PXD, ECL
#    phase          2, 31 (= early phase 3) or 3
#    sad            SAD file name from /home/belle/luka/public/SAD without bg type and ".root"
#    outdir         output directory path
# -------------------------------------------------------------------------------------

import basf2 as b2
import sys
import os
from background import add_output

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 5:
    bgType = argvs[1]     # Coulomb, Touschek, Brems
    accRing = argvs[2]    # LER or HER
    equivTime = argvs[3]  # equivalent SuperKEKB running time in micro-seconds
    num = argvs[4]        # output file number
    sampleType = 'usual'  # study, usual, PXD, ECL
    phase = 3             # 2 or 3
    sad = 'phase2.1.4_collimators_1'
    outdir = 'output'     # output directory path
elif argc == 6:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = 3
    sad = 'phase2.1.4_collimators_1'
    outdir = 'output'
elif argc == 7:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = int(argvs[6])
    sad = 'phase2.1.4_collimators_1'
    outdir = 'output'
elif argc == 8:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = int(argvs[6])
    sad = argvs[7]
    outdir = 'output'
elif argc == 9:
    bgType = argvs[1]
    accRing = argvs[2]
    equivTime = argvs[3]
    num = argvs[4]
    sampleType = argvs[5]
    phase = int(argvs[6])
    sad = argvs[7]
    outdir = argvs[8]
else:
    print('usage:')
    print('basf2', argvs[0],
          '(Touschek,Coulomb,Brems) (HER,LER) equivTime_us num [(study,usual,ECL,PXD) phase outdir]')
    sys.exit()


inputSAD = "/group/belle2/BGcampaigns/SAD/forG4/"
bgType = bgType + '_' + accRing
sadFile = inputSAD + bgType + '_' + sad + '.root'
realTime = float(equivTime) * 1000
fname = bgType + '_' + sampleType + '-phase' + str(phase) + '-' + num
outputFile = outdir + '/' + fname + '.root'

# check for the existance of a SAD file

if not os.path.exists(sadFile):
    b2.B2ERROR('SAD file ' + sadFile + ' not found')
    sys.exit()

# make output directory if it doesn't exist

if not os.path.exists(outdir):
    os.makedirs(outdir)

# log message
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

# BG generator
generator = b2.register_module('BeamBkgGenerator')
generator.param('fileName', sadFile)
generator.param('ringName', accRing)
generator.param('realTime', realTime)
main.add_module(generator)

# Geant geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
if phase == 31:
    geometry.param('additionalComponents', ['BEAMABORT', 'MICROTPC', 'CLAWS', 'HE3TUBE'])
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

# to not store MCParticles use this, if you want MCParticles comment these two lines and uncomment the line after
excludeBranch = ['MCParticles']
add_output(main, bgType, realTime, sampleType, phase, fileName=outputFile, excludeBranches=excludeBranch)


# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
