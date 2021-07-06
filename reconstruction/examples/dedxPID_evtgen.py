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
# This steering file generates several events and
# does a simple dE/dx measurement with path length
# correction. The results are stored in a ROOT file.
#
# Usage: basf2 dedxPID_evtgen.py
#
# Input: None
# Output: dedxPID_evtgen.root
#############################################################

import basf2 as b2
from tracking import add_tracking_reconstruction

# change to True if you want to use PXD hits (fairly small benefit, if any)
use_pxd = False

# register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')

# --------------------
# generation
# --------------------

eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [50])
eventinfoprinter = b2.register_module('EventInfoPrinter')

# create geometry
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('excludedComponents', ['EKLM'])

# EvtGen to provide generic BB events
evtgeninput = b2.register_module('EvtGenInput')


# --------------------
# simulation
# --------------------

g4sim = b2.register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = b2.LogLevel.ERROR


# -------------------
# dE/dx reconstruction
# --------------------

# set up the CDC dE/dx module
cdcdedx = b2.register_module('CDCDedxPID')
cdcdedx_params = {  # 'pdfFile': 'YourPDFFile.root',
    'useIndividualHits': True,
    'removeLowest': 0.05,
    'removeHighest': 0.25,
    'onlyPrimaryParticles': False,
    'enableDebugOutput': True,
    'ignoreMissingParticles': False,
}
cdcdedx.param(cdcdedx_params)

# set up the VXD dE/dx module
svddedx = b2.register_module('VXDDedxPID')
svddedx_params = {  # 'pdfFile': 'YourPDFFile.root',
    'useIndividualHits': True,
    'removeLowest': 0.05,
    'removeHighest': 0.25,
    'onlyPrimaryParticles': False,
    'usePXD': use_pxd,
    'useSVD': True,
    'trackDistanceThreshold': 4.0,
    'enableDebugOutput': True,
    'ignoreMissingParticles': False,
}
svddedx.param(svddedx_params)

# write the results to file
output = b2.register_module('RootOutput')
output.param('outputFileName', 'dedxPID_evtgen.root')


# create paths
main = b2.create_path()


# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(evtgeninput)
main.add_module(g4sim)

if use_pxd:
    main.add_module(b2.register_module('PXDDigitizer'))
    main.add_module(b2.register_module('PXDClusterizer'))
main.add_module(b2.register_module('SVDDigitizer'))
main.add_module(b2.register_module('SVDClusterizer'))
main.add_module(b2.register_module('CDCDigitizer'))

# tracking reconstruction does not include dE/dx measurements
add_tracking_reconstruction(main)

main.add_module(cdcdedx)
main.add_module(svddedx)

main.add_module(output)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
