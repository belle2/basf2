#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import os
import sys
import math
import string
import datetime

d = datetime.datetime.today()
print d.strftime('job start: %Y-%m-%d %H:%M:%S\n')

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 3:
    name = argvs[1]
    num = argvs[2]
else:
    print './RunBGMC.py [(RBB,Touschek,Coulomb)_(HER,LER)(,_far)] [num]'
    sys.exit()

# set accring (0:LER, 1:HER)
if name.find('LER') != -1:
    accring = 0
elif name.find('HER') != -1:
    accring = 1
else:
    print 'name should include either of HER or LER'
    sys.exit()

## set readmode (0:RBB, 1:Toucshek/Coulomb/Brems)
# if name.find("RBB") != -1 :
#    readmode = 0
# else
#    readmode = 1

# set range (0:RBB, 1:Toucshek/Coulomb/Brems)
if name.find('far') != -1:
    range = 2800
else:
    range = 400

inputfilename = 'forIgal/' + name + '.root'
logfilename = 'log/' + name + '_' + num + '.log'
outputfilename = 'output/phase1_' + name + '_' + num + '.root'
seed = '1' + num + num + '1'

# readouttime  [ns]
readouttime = 1000  # [ns]
nevent = 10000000
# overwrite below
# note that nevent for RBB sample should not exceed the
# number of nevent included in the input root file.

# readmode 0 is un-weighted
# readmode 1 is weighted

if name == 'Brems_LER':
    readmode = 1
elif name == 'Touschek_LER':
#    nevent=583 #1us
    readmode = 1
elif name == 'Coulomb_LER':
#    nevent=3985
    readmode = 1
elif name == 'Brems_HER':
#    nevent=1473
    readmode = 1
elif name == 'Touschek_HER':
#    nevent=2104 #1us
    readmode = 1
elif name == 'Coulomb_HER':
#    nevent=4257
    readmode = 1
else:
#    nevent=4489

    print 'Unknown name!'
    sys.exit()

print 'accring: ', accring, '(0:LER, 1:HER)'
print 'input:   ', inputfilename
print 'log:     ', logfilename
print 'output:  ', outputfilename
print 'range:   ', range
print 'seed:    ', seed
print 'nevent:  ', nevent
print 'readouttime:', readouttime

# -----------------------------------------------------------------

########################################################
# This steering file shows all options for the
# simulation of the Touschek effect.
#
# Please note:
# The Touschek input file contains weighted particles, in
# the following called 'SAD particles'.
# Thus, one particle in the input file doesn't correspond
# to one 'real' particle in an event. In order to simulate
# the Touschek effect for your subdetector, you have
# two possibilities:
#
# a) Create unweighted events by setting the 'ReadMode' to 1
#    and 'ReadoutTime' to the correct readout time of your
#    subdetector. Then run over ALL events in the input file.
#    The result will be a ROOT file containing all particles
#    which hit your subdetector during one readout frame/cycle
#    of your subdetector.
#
# b) Create weighted events by setting the 'ReadMode' to 0.
#    Run over ALL events in the input file. Each event will
#    contain one MonteCarlo track, carrying the weight
#    information. Using this information you can then scale
#    the result to your subdetector readout time.
#
# Which one you choose depends on the background studies
# you would like to perform. For example, if you are interested
# in the details of your subdetector occupancy, it is recommended
# to choose a). On the other hand, if you are interested in the
# flux or rate of the background hitting your subdetector, you can
# choose b).
#
# Example steering file - 2011 Belle II Collaboration
########################################################

## Set the global log level
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.WARNING)
# set_log_level(LogLevel.DEBUG)

set_random_seed(int(seed))

## Register the event meta generator and set the number of events to a very
## high number which exceeds the number of events in the input file.
## Register the event meta generator and set the number of events to a very
## high number which exceeds the number of events in the input file.
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevent], 'runList': [1], 'expList': [1]})

## Register the TouschekSADInput module and specify the location of the Touschek
## input file. The file can be downloaded from the TWiki.
touschekinput = register_module('SADInput')
touschekinput.param('Filename', inputfilename)

# Set the ReadMode of the TouschekSAD input module
# 0 = one SAD particle per event. This produces weighted events.
# 1 = one real particle per event. This produces unweighted events.
# 2 = all SAD particles in one event. Can be used for visualization
touschekinput.param('ReadMode', readmode)

# Set the RingFlag of the TouschekSAD input module
# 0 = LER
# 1 = HER
touschekinput.param('AccRing', accring)

# Set the readout time for your subdetector in [ns]. The value given
# here corresponds to the readout time of the PXD.
# This setting is only used if the 'ReadMode' is set to 1.
touschekinput.param('ReadoutTime', readouttime)

# Set the range around the IP in which SAD particles are accepted
# into the simulation. The value given below is highly recommended.
touschekinput.param('Range', range)

# If you would like to see some information about the created particles
# set the logging output of the Touschek Input module to DEBUG.
# touschekinput.set_log_level(LogLevel.DEBUG)

## Register the standard chain of modules to the framework,
## which are required for the simulation.
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
fullsim = register_module('FullSim')

param_fullsim = {'RegisterOptics': 1, 'PhotonFraction': 0.3}
fullsim.param(param_fullsim)
fullsim.param('PhysicsList', 'QGSP_BERT_HP')
fullsim.param('UICommands', ['/process/inactivate nKiller'])

# If you want to store all secondaries in MCParticles, use following lines.
# Default is False and 1MeV cut.
fullsim.param('StoreAllSecondaries', True)
fullsim.param('SecondariesEnergyCut', 0.0)  # in MeV

# fullsim.set_log_level(LogLevel.DEBUG)

## Add additional modules according to your own needs
# pxddigi   = register_module('PXDDigitizer')
progress = register_module('Progress')

## Write the output to a file
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', outputfilename)
rootoutput.param('updateFileCatalog', False)
# rootoutput.param('branchNames', ["BeamBackHits"])
# output.param('branchNames', ["BeamBackHits","EKLMStepHits"])

## Create the main path and add the required modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(touschekinput)
main.add_module(geometry)
main.add_module(fullsim)
main.add_module(progress)
main.add_module(rootoutput)

process(main)

### Print some basic event statistics
print 'Event Statistics:'
print statistics

d = datetime.datetime.today()
print d.strftime('job finish: %Y-%m-%d %H:%M:%S\n')
