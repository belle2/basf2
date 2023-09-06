#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from tracking import add_tracking_reconstruction
from simulation import add_simulation

b2.set_random_seed(1)

# Create paths
main = b2.create_path()

# Add modules to paths
main.add_module('EventInfoSetter', expList=[0], runList=[1], evtNumList=[100])
main.add_module('EvtGenInput', logLevel=b2.LogLevel.INFO)

add_simulation(main, components=['PXD', 'SVD', 'CDC'], forceSetPXDDataReduction=True, usePXDDataReduction=False)
add_tracking_reconstruction(main, ['SVD', 'CDC'])

main.add_module('PXDROIFinder',
                recoTrackListName='RecoTracks',
                PXDInterceptListName='PXDIntercepts',
                ROIListName='ROIs',
                logLevel=b2.LogLevel.DEBUG)
# PXD digitization module
main.add_module('PXDDigitizer')
# PXD clusterizer
main.add_module('PXDClusterizer')
# filter PXDClusters that are in ROIs
main.add_module('PXDclusterFilter', ROIidsName='ROIs', CreateOutside=True, overrideDB=True, enableFiltering=True)

main.add_module('Progress')

# Process events
b2.process(main)

print(b2.statistics)
