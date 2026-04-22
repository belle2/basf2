#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------
# Example steering script that generates anti-neutron events with fixed momentum
# and applies the TOPBackSplashTiming module
# ---------------------------------------------------------------------------------

import basf2 as b2
import simulation as si
import reconstruction as re
from generators import add_evtgen_generator
import glob

# bg = glob.glob('/group/belle2/dataprod/BGOverlay/run2/prerelease-08-00-00a/new_overlay/BGx1/set2/BGforOverlay-6*.root')
bg = glob.glob('/group/belle2/dataprod/BGOverlay/run2/prerelease-08-00-00a/new_overlay/BGx1/set?/*.root')

b2.conditions.prepend_globaltag('patch_main_release-10')
b2.conditions.prepend_globaltag('patch_main_release-11')

path = b2.create_path()

path.add_module("EventInfoSetter", evtNumList=1000, expList=1004)
path.add_module("EventInfoPrinter")

# EvtGen
add_evtgen_generator(path=path, finalstate='mixed')


# detector and L1 trigger simulation and reco
si.add_simulation(path=path,  bkgfiles=bg)
re.add_reconstruction(path=path, enable_top_cluster_timing=True)


# Save mdst with timing, no. of fitted photons and chi-2/dof
path.add_module("RootOutput",
                outputFileName="output_TOPBackSplashTiming.root",
                )

# run
path.add_module("Progress")


# generate events
b2.process(path, calculateStatistics=True)

# show call statistics
print(b2.statistics)
