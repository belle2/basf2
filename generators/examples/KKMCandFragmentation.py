#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Steering for the new continuum production scheme,
# running KKMC and PYTHIA in a single job.
#
# Produces ee->uubar(g) events. For dd, ss or cc you have to
# change the kkmc_inputfile.
#
# Author(s): Martin Ritter (martin.ritter@lmu.de)
#            Torben Ferber (ferber@physics.ubc.ca)
#
########################################################

from basf2 import *
import ROOT
from ROOT import Belle2
import multiprocessing
import tempfile
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output

# Suppress messages and warnings during processing.
set_log_level(LogLevel.RESULT)

#: number of events to generate, can be overriden with -n
num_events = 1000
#: output filename, can be overriden with -o
output_filename = "RootOutput.root"

#: kkmc input file, one for each qqbar mode
kkmc_inputfile = Belle2.FileSystem.findFile('data/generators/kkmc/uubar_nohadronization.input.dat')
#: pythia configuration
pythia_config = Belle2.FileSystem.findFile('data/generators/modules/fragmentation/pythia_belle2.dat')
#: user decay file
decay_user = Belle2.FileSystem.findFile('data/generators/modules/fragmentation/dec_belle2_qqbar.dec')

#: kkmc configuration file, should be fine as is
kkmc_config = Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat')

main = create_path()
# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=num_events)
# use KKMC to generate uubar events (no fragmentation at this stage)
kkgeninput = main.add_module(
    'KKGenInput',
    tauinputFile=kkmc_inputfile,
    KKdefaultFile=kkmc_config,
    taudecaytableFile='',
    kkmcoutputfilename='kkmc_uu.txt',
)

# add the fragmentation module to fragment the generated quarks into hadrons
# using PYTHIA
fragmentation = main.add_module(
    'Fragmentation',
    ParameterFile=pythia_config,
    ListPYTHIAEvent=0,
    UseEvtGen=1,
    UserDecFile=decay_user,
)

# branch to an empty path if PYTHIA failed, this will change the number of events
# but the file meta data will contain the total number of generated events
emptypath = create_path()
fragmentation.if_value('<1', emptypath)

# add simulation
add_simulation(main)

# add reconstruction
add_reconstruction(main)

# Finally add mdst output, name is chosen to be consistent with default
# RootOutput filename so that fixing of metadata later works
add_mdst_output(main, filename=output_filename)

# optionally, print the MCParticles
# main.add_module("PrintMCParticles", logLevel=LogLevel.INFO)

# and process the job
process(main)
