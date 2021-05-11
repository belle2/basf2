#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import basf2 as b2
from beamparameters import add_beamparameters

numEvents = 25
initialValue = 42

b2.set_log_level(b2.LogLevel.ERROR)
b2.set_random_seed(initialValue)

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = b2.register_module('EventInfoPrinter')

gearbox = b2.register_module('Gearbox')

pxdDigitizer = b2.register_module('PXDDigitizer')
svdDigitizer = b2.register_module('SVDDigitizer')
pxdClusterizer = b2.register_module('PXDClusterizer')
svdClusterizer = b2.register_module('SVDClusterizer')
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.logging.log_level = b2.LogLevel.WARNING

geometry = b2.register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])

g4sim = b2.register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

spCreatorSingle = b2.register_module('SVDSpacePointCreator')
spCreatorSingle.logging.log_level = b2.LogLevel.INFO
spCreatorSingle.logging.debug_level = 1
spCreatorSingle.param('OnlySingleClusterSpacePoints', True)
spCreatorSingle.param('NameOfInstance', 'singlesOnly')
spCreatorSingle.param('SpacePoints', 'singlesOnly')

spCreatorCombi = b2.register_module('SVDSpacePointCreator')
spCreatorCombi.logging.log_level = b2.LogLevel.DEBUG
spCreatorCombi.logging.debug_level = 1
spCreatorCombi.param('OnlySingleClusterSpacePoints', False)
spCreatorCombi.param('NameOfInstance', 'couplesAllowed')
spCreatorCombi.param('SpacePoints', 'couplesAllowed')

spCreatorPXD = b2.register_module('PXDSpacePointCreator')
spCreatorPXD.logging.log_level = b2.LogLevel.INFO
spCreatorPXD.logging.debug_level = 1
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
spCreatorPXD.param('SpacePoints', 'pxdOnly')

spCreatorTest = b2.register_module('SpacePointCreatorTest')
spCreatorTest.logging.log_level = b2.LogLevel.DEBUG
spCreatorTest.logging.debug_level = 20
spCreatorTest.param('NameOfInstance', 'SPTester')
# spCreatorTest.param('SpacePoints', 'couplesAllowed')
# spCreatorTest.param('AllSpacePointContainers', [ 'pxdOnly'] )
spCreatorTest.param('AllSpacePointContainers', ['singlesOnly', 'couplesAllowed', 'pxdOnly'])

# Create paths
main = b2.create_path()


# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters = add_beamparameters(main, "Y1S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
# beamparameters.param("smearEnergy", False)
# print_params(beamparameters)


main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(spCreatorSingle)
main.add_module(spCreatorCombi)
main.add_module(spCreatorPXD)
main.add_module(spCreatorTest)

# Process events
b2.process(main)

print('Event Statistics :')
print(b2.statistics)
