##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Path building
from basf2 import create_path, process, statistics
from simulation import add_simulation
from tracking import add_prefilter_tracking_reconstruction

path = create_path()

path.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=10)

# generate BBbar events
path.add_module('EvtGenInput')

# detector simulation, don't perfrom PXD data reduction
add_simulation(path, bkgOverlay=False, forceSetPXDDataReduction=True, usePXDDataReduction=False, cleanupPXDDataReduction=False)

# add tracking and track fitting, without MC matching
add_prefilter_tracking_reconstruction(path)

# Add the Chi2-matcher to path
# We need CutOffs values, which are from a preliminary optimisation
# In addition, a package used for inversion of the covariance matrix is needed, which is set by the linalg parameter
# ROOT is default since in general it is faster and scales better
# False: ROOT [default]; True: eigen
# Add the Chi2-matcher module to the execution path
path.add_module("Chi2MCTrackMatcher", CutOffs=[128024, 95, 173, 424, 90, 424], linalg=False)
# process the path
process(path)
# show some module statistics
print(statistics)
