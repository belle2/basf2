#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This example uses a python module to show the ParticleGun vertex
# mean and sigma in the display.

import basf2 as b2

from ROOT import Belle2
from ROOT import TVector3

# Setting the parameters for random generation of the event vertex Three
# different distributions can be used: - fixed:     always use the exact same
# value - uniform:   uniform distribution between min and max - normal: normal
# distribution around mean with width of sigma
#
# The default is a normal distribution of the vertex

# X/Y/Z parameters:
vertexparams = [[0, 1], [0, 1], [10, 2]]


class VisualizeVertex(b2.Module):
    """Visualize the vertex configured for the ParticleGun"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj("DisplayData")
        displayData.create()

        displayData.obj().addLabel("Origin", TVector3(0, 0, 0))
        displayData.obj().addLabel("0,0,10", TVector3(0, 0, 10))

        # visualize extent of vertex
        global vertexparams
        displayData.obj().addPoint("Vertex", TVector3(vertexparams[0][0],
                                                      vertexparams[1][0],
                                                      vertexparams[2][0]))
        for d in range(3):
            sigma = vertexparams[d][1]
            # add points at +-sigma on both sides of Vertex
            vertexparams[d][0] -= sigma
            displayData.obj().addPoint("Width", TVector3(vertexparams[0][0],
                                                         vertexparams[1][0],
                                                         vertexparams[2][0]))
            vertexparams[d][0] += 2 * sigma
            displayData.obj().addPoint("Width", TVector3(vertexparams[0][0],
                                                         vertexparams[1][0],
                                                         vertexparams[2][0]))
            vertexparams[d][0] -= sigma


# register necessary modules
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [500])


# create geometry
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('components', ['CDC', 'MagneticField'])

particlegun = b2.register_module('ParticleGun')

particlegun.param('vertexGeneration', 'normal')

particlegun.param('xVertexParams', vertexparams[0])
particlegun.param('yVertexParams', vertexparams[1])
particlegun.param('zVertexParams', vertexparams[2])

# simulation
g4sim = b2.register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = b2.LogLevel.ERROR

# create paths
main = b2.create_path()

# add modules to paths
main.add_module(eventinfosetter)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
# main.add_module(g4sim)

main.add_module(VisualizeVertex())

# default parameters
display = b2.register_module('Display')
display.param('showAllPrimaries', True)
main.add_module(display)

b2.process(main)
print(b2.statistics)
