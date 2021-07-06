#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>BabayagaNLOBhabhaSimRec.root</output>
  <cacheable/>
  <contact>Software team b2soft@mail.desy.de</contact>
  <description>
    This steering file produces 1000 radiative Bhabha events with
    Babayaga.NLO, runs the detector simulation with mixed in background, and
    performs the standard reconstruction.
  </description>
</header>
"""

from basf2 import set_random_seed, create_path, process, statistics
from simulation import add_simulation
from reconstruction import add_reconstruction
from validation import statistics_plots, event_timing_plot
from background import get_background_files

set_random_seed(12345)

main = create_path()
emptypath = create_path()

# specify number of events to be generated
main.add_module("EventInfoSetter", evtNumList=[1000], runList=[1], expList=[0])

# generate Bhabha events
main.add_module("BabayagaNLOInput")

# register the preselection module
generatorpreselection = main.add_module("GeneratorPreselection")
generatorpreselection.param(
    {
        "nChargedMin": 1,
        "MinChargedP": 0.5,
        "MinChargedPt": 0.1,
        "MinChargedTheta": 16.5,
        "MaxChargedTheta": 150.5,
        "nPhotonMin": 1,
        "MinPhotonEnergy": 0.5,
        "MinPhotonTheta": 12.0,
        "MaxPhotonTheta": 156.0,
    }
)
generatorpreselection.if_value("<1", emptypath)

# detector simulation
add_simulation(main, bkgfiles=get_background_files())

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module("Profile")

# output
main.add_module("RootOutput", outputFileName="../BabayagaNLOBhabhaSimRec.root")

process(main)

# Print call statistics
print(statistics)

statistics_plots(
    "BabayagaNLOBhabhaSimRec_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation and reconstruction job with radiative "
    "Bhabha events using Babayaga.NLO",
    prefix="BabayagaNLOBhabhaSimRec",
)
event_timing_plot(
    "../BabayagaNLOBhabhaSimRec.root",
    "BabayagaNLOBhabhaSimRec_statistics.root",
    contact="Software team b2soft@mail.desy.de",
    job_desc="a standard simulation and reconstruction job with radiative "
    "Bhabha events using Babayaga.NLO",
    prefix="BabayagaNLOBhabhaSimRec",
)
