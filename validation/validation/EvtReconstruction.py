#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>EvtGenSim.root</input>
  <output>EvtRec.root,EvtRec_mdst.root</output>
  <cacheable/>
  <contact>arul.prakash@physik.uni-muenchen.de</contact>
  <description>
    This steering file runs the standard reconstruction on an input file with
    generic BBbar events.
  </description>
</header>
"""

from basf2 import set_random_seed, create_path, process
from reconstruction import add_reconstruction
from mdst import add_mdst_output
from validation import statistics_plots, event_timing_plot

set_random_seed(12345)

main = create_path()

# read file of simulated events
main.add_module("RootInput", inputFileName="../EvtGenSim.root")

# geometry parameter database
main.add_module("Gearbox")

# detector geometry
main.add_module("Geometry")

# reconstruction
add_reconstruction(main)

# memory profile
main.add_module("Profile")

# output
main.add_module("RootOutput", outputFileName="../EvtRec.root")
add_mdst_output(main, True, "../EvtRec_mdst.root")

main.add_module('Progress')
process(main, calculateStatistics=True)

statistics_plots(
    "EvtRec_statistics.root",
    contact="arul.prakash@physik.uni-muenchen.de",
    job_desc="a standard reconstruction job with generic EvtGen events",
    prefix="EvtRec",
)
event_timing_plot(
    "../EvtRec.root",
    "EvtRec_statistics.root",
    contact="arul.prakash@physik.uni-muenchen.de",
    job_desc="a standard reconstruction job with generic EvtGen events",
    prefix="EvtRec",
)
