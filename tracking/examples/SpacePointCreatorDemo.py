#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This steering file will
import basf2 as b2
from beamparameters import add_beamparameters
from simulation import add_simulation

b2.set_log_level(b2.LogLevel.ERROR)
b2.set_random_seed(42)

# Create paths
main = b2.create_path()

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")

main.add_module('EventInfoSetter', expList=[0], runList=[1], evtNumList=[10])
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('EvtGenInput', logLevel=b2.LogLevel.WARNING)
# For this example we only need PXD and SVD in simulation, and we only need to clusterize their data, too
add_simulation(main, forceSetPXDDataReduction=True, usePXDDataReduction=False)
main.add_module('PXDClusterizer')
main.add_module('SVDClusterizer')
main.add_module('SVDSpacePointCreator', OnlySingleClusterSpacePoints=True, NameOfInstance='singlesOnly',
                SpacePoints='singlesOnly', logLevel=b2.LogLevel.INFO).set_name('SingleSideSVDSpacePointCreator')
main.add_module(
    'SVDSpacePointCreator',
    OnlySingleClusterSpacePoints=False,
    NameOfInstance='couplesAllowed',
    SpacePoints='couplesAllowed',
    logLevel=b2.LogLevel.DEBUG,
    debugLevel=1).set_name('BothSideSVDSpacePointCreator')
main.add_module('PXDSpacePointCreator', NameOfInstance='pxdOnly', SpacePoints='pxdOnly', logLevel=b2.LogLevel.INFO)
main.add_module(
    'SpacePointCreatorTest',
    NameOfInstance='SPTester',
    AllSpacePointContainers=[
        "singlesOnly",
        "couplesAllowed",
        "pxdOnly"],
    logLevel=b2.LogLevel.DEBUG,
    debugLevel=20)
main.add_module('Progress')

for m in main.modules():
    if m.name() == "FullSim":
        m.param('StoreAllSecondaries', True)

# Process events
b2.process(main)

print('Event Statistics :')
print(b2.statistics)
