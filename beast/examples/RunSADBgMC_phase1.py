#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
import sys
import datetime


class PyTrigger(b2.Module):

    """Returns 1 if current event contains at least one BEAST hit, 0 otherwise"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

    def event(self):
        """reimplementation of Module::event()."""

        self.return_value(0)
        mcparticles = Belle2.PyStoreArray('MCParticles')
        for p in mcparticles:
            print()
            if len(p.getRelationsTo('MicrotpcSimHits')) > 0 \
                    or len(p.getRelationsTo('He3tubeSimHits')) > 0 \
                    or len(p.getRelationsTo('PindiodeSimHits')) > 0 \
                    or len(p.getRelationsTo('BgoSimHits')) > 0 \
                    or len(p.getRelationsTo('CsiSimHits')) > 0:
                #                B2INFO('found a Beast!')
                self.return_value(1)

                break


d = datetime.datetime.today()
print(d.strftime('job start: %Y-%m-%d %H:%M:%S\n'))

# read parameters

argvs = sys.argv
argc = len(argvs)

if argc == 3:
    name = argvs[1]
    num = argvs[2]
else:
    print('./RunBGMC.py [(RBB,Touschek,Coulomb)_(HER,LER)(,_far)] [num]')
    sys.exit()

# set accring (0:LER, 1:HER)
if name.find('LER') != -1:
    accring = 0
elif name.find('HER') != -1:
    accring = 1
else:
    print('name should include either of HER or LER')
    sys.exit()

# set readmode (0:RBB, 1:Toucshek/Coulomb/Brems)
# if name.find("RBB") != -1 :
#    readmode = 0
# else
#    readmode = 1

# set range (0:RBB, 1:Toucshek/Coulomb/Brems)
if name.find('far') != -1:
    range = 2800
else:
    range = 400

# inputfilename = 'phase1_fullsim2/' +  name  + '.root'
inputfilename = 'forIgal/' + name + '.root'
logfilename = 'log/' + name + '_' + num + '.log'
outputfilename = 'output/out_phase1_pos_TiN_' + name + '_' + num + '.root'
seed = '1' + num + num + '1'

# readouttime  [ns]
# good readouttime = 100000 #[ns]
readouttime = 100000  # [ns]
# readouttime = 1000 #[ns]
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

    print('Unknown name!')
    sys.exit()

print('accring: ', accring, '(0:LER, 1:HER)')
print('input:   ', inputfilename)
print('log:     ', logfilename)
print('output:  ', outputfilename)
print('range:   ', range)
print('seed:    ', seed)
print('nevent:  ', nevent)
print('readouttime:', readouttime)

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

# Set the global log level
# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.WARNING)
# set_log_level(LogLevel.DEBUG)

b2.set_random_seed(int(seed))

# Register the event meta generator and set the number of events to a very
# high number which exceeds the number of events in the input file.
# Register the event meta generator and set the number of events to a very
# high number which exceeds the number of events in the input file.
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevent], 'runList': [1], 'expList': [1]})

# Register the TouschekSADInput module and specify the location of the Touschek
# input file. The file can be downloaded from the TWiki.
touschekinput = b2.register_module('SADInput')
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

# Register the standard chain of modules to the framework,
# which are required for the simulation.
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
gearbox.param('fileName', '/geometry/Beast2_phase1.xml')
fullsim = b2.register_module('FullSim')

param_fullsim = {'RegisterOptics': 1, 'PhotonFraction': 0.3}
fullsim.param(param_fullsim)
fullsim.param('PhysicsList', 'QGSP_BERT_HP')
fullsim.param('UICommandsAtIdle', ['/process/inactivate nKiller'])

# If you want to store all secondaries in MCParticles, use following lines.
# Default is False and 1MeV cut.
fullsim.param('StoreAllSecondaries', True)
fullsim.param('SecondariesEnergyCut', 0.0)  # in MeV

# fullsim.set_log_level(LogLevel.DEBUG)

# Add additional modules according to your own needs
progress = b2.register_module('Progress')

# Write the output to a file
rootoutput = b2.register_module('RootOutput')
rootoutput.param('outputFileName', outputfilename)
rootoutput.param('updateFileCatalog', False)
# rootoutput.param('branchNames', ["BeamBackHits"])
# routoutput.param('branchNames', ["BeamBackHits","EKLMStepHits"])
# rootoutput.param('branchNames', ["BgoSimHits","MicrotpcSimHits","PindiodeSimHits","He3tubeSimHits","BeamBackHits","MCParticles"])
# rootoutput.param('branchNames', ["BgoSimHits","MicrotpcSimHits","PindiodeSimHits","He3tubeSimHits","CsiSimHits","MCParticles"])
# rootoutput.param('branchNames', ["BgoSimHits","MicrotpcSimHits","PindiodeSimHits","He3tubeSimHits","CsiSimHits"])
# rootoutput.param('branchNames', ["BgoSimHits","MicrotpcSimHits","PindiodeSimHits","He3tubeSimHits","CsiSimHits","MCParticles"])
rootoutput.param('branchNames', ['BgoSimHits', 'MicrotpcSimHits',
                                 'PindiodeSimHits', 'He3tubeSimHits', 'CsiSimHits'])

# Create the main path and add the required modules
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(touschekinput)
main.add_module(geometry)
main.add_module(fullsim)
main.add_module(progress)
########################################
# simulation is done now, so we'll put the PyTrigger module here
# trigger = PyTrigger()
# main.add_module(trigger)

# if PyTrigger returns 0, we'll jump into an empty path
# (skipping further modules in 'main': Beast2 SimHits)
# emptypath = create_path()
# trigger.if_false(emptypath)
########################################
main.add_module(rootoutput)

# he3digi = register_module('He3Digitizer')
# main.add_module(he3digi)
# sampletime=10000
# he3tube = register_module('He3tube')
# he3tube.param('sampleTime', sampletime);
# main.add_module(he3tube)

b2.process(main)

# Print some basic event statistics
print('Event Statistics:')
print(b2.statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
