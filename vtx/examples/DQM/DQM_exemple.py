#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# Simple steering file to demonstrate how to run DQM on BelleII geometry
#############################################################

import basf2 as b2
from simulation import add_simulation
# from reconstruction import add_reconstruction
from reconstruction import add_mc_reconstruction

import glob

# use globaltag or xmlfiles
use_globaltag = True

# background (collision) files
bg = None

# number of events to generate, can be overriden with -n
num_events = 20

# output filename, can be overriden with -o
output_filename = "VTXRootOutput.root"


# Need to use default global tag prepended with upgrade GT
if use_globaltag:
    from vtx import get_upgrade_globaltag
    b2.conditions.disable_globaltag_replay()
    b2.conditions.prepend_globaltag(get_upgrade_globaltag())


# create path
main = b2.create_path()

main.add_module("EventInfoSetter", evtNumList=num_events)


# histomanager
Histos_filename = "Histos_DQMVTX.root"
main.add_module('HistoManager', histoFileName=Histos_filename)

main.add_module('EvtGenInput')

main.add_module('Gearbox')

if use_globaltag:
    main.add_module('Geometry')

else:
    main.add_module('Geometry', excludedComponents=['PXD', 'SVD'],
                    additionalComponents=['VTX-CMOS-7layer'],
                    useDB=False)

# detector simulation
add_simulation(main, bkgfiles=bg, useVTX=True)

# reconstruction
add_mc_reconstruction(main, pruneTracks=False, useVTX=True)


main.add_module('VTXDQMClusters', histogramDirectoryName='VTXCls')

main.add_module('VTXDQMExpressReco')

# output if needed
# main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
main.add_module('Progress')
b2.process(main)
print(b2.statistics)
