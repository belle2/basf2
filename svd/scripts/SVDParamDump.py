#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2

logging.log_level = LogLevel.WARNING


# ------------------------------------------------------------------------------
# Analysis module that dumps parameters of SVD sensors
# ------------------------------------------------------------------------------

class DumpSVDSensorParameters(Module):

    """A module to dump sensor data of SVD sensors."""

    def __init__(self):
        """Initialize the module"""

        super(DumpSVDSensorParameters, self).__init__()
        #: Input file object.
        self.file = open('DumpSVDSensorParametersData.txt', 'w')

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('sensor_type layer ladder sensor side pitch strip_length ')
        self.file.write('c_backp c_inter c_coup noise\n')

    def event(self):
        """Get values of sensor parameters from GeoCache and write them to file."""

        geoCache = Belle2.VXD.GeoCache.getInstance()
        layer_sensors = {3: 2, 4: 3, 5: 4, 6: 5}
        for sensorID in geoCache.getListOfSensors():
            info = Belle2.SVD.SensorInfo(geoCache.getSensorInfo(sensorID))
            # Now let's store some data
            # Sesnor identification
            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()
            sensorType = 'layer3' if layer == 3 else ('slanted' if sensor == 1 else 'barrel')
            origami = 0 if sensor == layer_sensors[layer] else 1
            for side in ['u', 'v']:
                s = '{sID} {orig} {layer} {ladder} {sensor} {uv} '.format(
                    sID=sensorType,
                    orig=origami,
                    layer=layer,
                    ladder=ladder,
                    sensor=sensor,
                    uv=side
                )
                # Parameter information
                pitch = info.getUPitch() if side == 'u' else info.getVPitch()
                halfCellU = info.getUCells()
                halfCellV = info.getVCells()
                strip_length = info.getStripLengthU(halfCellU) if side == 'u' else info.getStripLengthV(halfCellV)
                capa_bp = info.getBackplaneCapacitanceU() if side == 'u' else info.getBackplaneCapacitanceV()
                capa_inter = info.getInterstripCapacitanceU() if side == 'u' else info.getInterstripCapacitanceV()
                capa_coup = info.getCouplingCapacitanceU() if side == 'u' else info.getCouplingCapacitanceV()
                noise = info.getElectronicNoiseU() if side == 'u' else info.getElectronicNoiseV()
                s += '{p:10.5f} {sl:10.5f} {cbp:10.5f} {cis:10.5f} {ccp:10.5f} {sig:10.5f}\n '.format(
                    p=pitch,
                    sl=strip_length,
                    cbp=capa_bp,
                    cis=capa_inter,
                    ccp=capa_coup,
                    sig=noise
                )
                self.file.write(s)

    def terminate(self):
        """ Close the output file."""

        self.file.close()

# ------------------------------------------------------------------------------

# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# SVD digitization module
svddigi = register_module('SVDDigitizer')
# SVD clustering module
svdclust = register_module('SVDClusterizer')
# RootOutput
output = register_module('RootOutput')
# Analysis module
analyze = DumpSVDSensorParameters()

# Specify number of events to generate
eventinfosetter.param('evtNumList', [1])

# Set parameters for particlegun
particlegun.param({
    'nTracks': 1,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [3, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniform',
    'thetaParams': [89, 91],
    'vertexGeneration': 'normal',
    'xVertexParams': [0.0, 0.1],
    'yVertexParams': [0.0, 0.1],
    'zVertexParams': [0.0, 5.0]
})

# Select subdetectors to be built
geometry.param('components', ['SVD'])

# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(svddigi)
main.add_module(svdclust)
main.add_module(analyze)

# generate events
process(main)

# show call statistics
print(statistics)
