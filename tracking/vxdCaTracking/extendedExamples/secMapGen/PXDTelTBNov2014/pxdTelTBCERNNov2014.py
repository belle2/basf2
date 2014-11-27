#!/usr/bin/env python
# -*- coding: utf-8 -*-

events = 1000  # Number of events to simulate
numTracks = 1
momentum = 120  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  ## degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

from basf2 import *
set_log_level(LogLevel.ERROR)
main = create_path()

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})
main.add_module(eventinfosetter)

# Load parameters from xml
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/vxd/CERN2014_v0.xml')
main.add_module(gearbox)

# Create geometry
geometry = register_module('Geometry')
main.add_module(geometry)

# ParticleGun
particlegun = register_module('ParticleGun')
particlegun.param('nTracks', numTracks)
particlegun.param('pdgCodes', [211, -211])  # pions +/-
particlegun.param('momentumGeneration', 'normal')
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.])
particlegun.param('yVertexParams', [0., beamspot_size_y])
particlegun.param('zVertexParams', [0., beamspot_size_z])
particlegun.param('independentVertices', True)
main.add_module(particlegun)

# Full simulation module
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)
# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])
main.add_module(simulation)

# ---------------------------------

fieldValue = 0.  # expected magnetic field in Tesla
# parameters for the secMap-calculation:
pTcuts = [1.]
setupFileName = 'SecMapCERN2014_120GeV'
secConfigU = [0., 1.]
secConfigV = [0., 1.]

filterCalc = register_module('FilterCalculator')
filterCalc.logging.log_level = LogLevel.DEBUG
filterCalc.logging.debug_level = 20
param_fCalc = {  # currently accepted: PXD, SVD, TEL
                 # 2?
    'detectorType': ['PXD', 'TEL'],
    'acceptedRegionForSensors': [-1, -1],
    'maxXYvertexDistance': 200.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 0,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 7,
    'sectorConfigU': secConfigU,
    'sectorConfigV': secConfigV,
    'setOrigin': [gun_x_position, 0., 0.],
    'magneticFieldStrength': fieldValue,
    'testBeam': 2,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileName + '_PXDEUTEL',
    'smearHits': 1,
    'uniSigma': 0.3,
    'noCurler': 1,
    }
filterCalc.param(param_fCalc)
main.add_module(filterCalc)

# using one export module only
exportXML = register_module('ExportSectorMap')
exportXML.logging.log_level = LogLevel.DEBUG
exportXML.logging.debug_level = 30
exportXML.param('rootFileName', setupFileName)
exportXML.param('sortByDistance2origin', True)

main.add_module(exportXML)

# -------------------------------------

# display = register_module('Display')
# display.param('fullGeometry', True)
# main.add_module(display)

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'output.root')
main.add_module(output)

# main.add_module(register_module('GBLdiagnostics'))

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Export used geometry for checking
geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'geometry.root')
main.add_module(geosaver)

process(main)

# Print call statistics
print statistics
