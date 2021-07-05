#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import ROOT
from ROOT import Belle2

b2.logging.log_level = b2.LogLevel.WARNING


class PrintPXDHits(b2.Module):

    """Prints global coordinates of PXD hits to demonstrate the Pythonized
    VXD::GeoCache.
    Only SensorInfoBase is Pythonized (not PXD:SensorInfo and SVD::SensorInfo),
    so that there is currently no access to specific PXD/SVD sensor features
    (like PXD bulk doping or SVD APV25 time constants - such support can be
    easily added, if desired, by placing the classes in the corresponding
    linkdef.h file.
    Complete geometry and sensor matrix information is, however, available
    for all sensor types.
     """

    def __init__(self):
        """Initialize the module"""

        super(PrintPXDHits, self).__init__()

    def initialize(self):
        """ Does nothing """

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Prints out PXD hits in global coordinates."""

        geoCache = Belle2.VXD.GeoCache.getInstance()
        # PXD clusters
        pxd_clusters = Belle2.PyStoreArray("PXDClusters")
        if pxd_clusters.getEntries() > 0:
            id = pxd_clusters[0].getSensorID()
            info = geoCache.get(id)
        else:
            b2.B2INFO("No PXD hits in this event.")

        for cluster in pxd_clusters:
            if id != cluster.getSensorID():  # next sensor
                id = cluster.getSensorID()
                info = geoCache.get(id)
                b2.B2INFO("Layer: {layer}, Ladder: {ladder}, Sensor: {sensor}"
                          .format(layer=id.getLayerNumber(),
                                  ladder=id.getLadderNumber(),
                                  sensor=id.getSensorNumber()))

            r_local = ROOT.TVector3(cluster.getU(), cluster.getV(), 0)
            r_global = info.pointToGlobal(r_local)
            b2.B2INFO('PXD hit: {x:10.5f} {y:10.5f} {z:10.5f}'
                      .format(x=r_global.X(), y=r_global.Y(), z=r_global.Z()))

    def terminate(self):
        """ Do nothing """


# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
# Show progress of processing
progress = b2.register_module('Progress')
# Load parameters
gearbox = b2.register_module('Gearbox')
# Create geometry
geometry = b2.register_module('Geometry')
# Run simulation
simulation = b2.register_module('FullSim')
# PXD digitization module
pxddigi = b2.register_module('PXDDigitizer')
# PXD clustering module
pxdclust = b2.register_module('PXDClusterizer')
# Print hits
printHits = PrintPXDHits()
printHits.set_log_level(b2.LogLevel.INFO)

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [5], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({  # Generate 5 tracks on average
                     # the number of tracks is a Poisson variate
                     # Generate pi+, pi-, e+ and e-
                     # with a normal distributed transversal momentum
                     # with a center of 5 GeV and a width of 1 GeV
                     # a normal distributed phi angle,
                     # center of 180 degree and a width of 30 degree
                     # Generate theta angles uniform in cos theta
                     # between 17 and 150 degree
                     # normal distributed vertex generation
                     # around the origin with a sigma of 2cm in the xy plane
                     # and no deviation in z
                     # all tracks sharing the same vertex per event
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normalPt',
    'momentumParams': [5, 1],
    'phiGeneration': 'normal',
    'phiParams': [0, 360],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [17, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 1],
    'yVertexParams': [0, 1],
    'zVertexParams': [0, 1],
    'independentVertices': False,
    })

# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'PXD'])

# create processing path
main = b2.create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(pxddigi)
main.add_module(pxdclust)
main.add_module(printHits)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
