#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from matplotlib.patches import Circle
import matplotlib.cm as colormap

from basf2 import *
from ROOT import Belle2

import os


def plot(x, y, col, show=0):
    """
    Plot a list of x/y values, plus CDC superlayer boundaries.

    Returns a pyplot.figure that can be saved.
    """
    fig = plt.figure(figsize=(8, 8))
    fig.subplots_adjust(bottom=0.1, left=0.1, right=0.95, top=0.95)
    ax = fig.add_subplot(111)

    # draw the x/y arrays. note that looping over the hits and
    # drawing them individually would be much slower
    for i in range(len(col)):
        ax.plot(
            x[i],
            y[i],
            marker='.',
            color=col[i],
            linestyle='None',
            markersize=1,
            )

    ax.set_title('CDCSimHits')
    ax.set_xlabel('x [cm]')
    ax.set_ylabel('y [cm]')
    ax.axis('scaled')

    # draw CDC superlayer boundaries
    layers = [
        16.8,
        25.7,
        36.5,
        47.6,
        58.4,
        69.5,
        80.2,
        91.3,
        102.0,
        111.1,
        ]
    Circs = [Circle((0, 0), a, facecolor='none', edgecolor='lightgrey')
             for a in layers]
    for e in Circs:
        ax.add_artist(e)

    ax.set_xlim(-130, 130)
    ax.set_ylim(-130, 130)
    if show:
        plt.show()
    return fig


class CDCPlotModule(Module):
    """An example python module.

    It gathers the x/y position off all CDCSimHits and draws them using
    matplotlib. The result is saved as a PNG.
    """

    ## event counter
    num_events = 0

    def __init__(self):
        """constructor."""
        super(CDCPlotModule, self).__init__()
        self.setName('CDCPlotModule')

    def event(self):
        """reimplementation of Module::event().

        loops over the CDCSimHits in the current event.
        """
        simhits = Belle2.PyStoreArray('CDCSimHits')

        trackhits_x = []
        trackhits_y = []
        indices = []

        mcparts = []
        for hit in simhits:
            hitpos = hit.getPosWire()  # TVector3
            # get index of first related mcparticle
            mcpart_idx = hit.getRelationsFrom("MCParticles")[0].getArrayIndex()
            if not mcpart_idx in mcparts:
                idx = len(mcparts)
                mcparts.append(mcpart_idx)
                indices.append(idx)
                trackhits_x.append([])
                trackhits_y.append([])
            # add simhit to the list corresponding to this particle
            idx = mcparts.index(mcpart_idx)
            trackhits_x[idx].append(hitpos.x())
            trackhits_y[idx].append(hitpos.y())

        if len(indices) > 0:
            # plot the (x,y) list on a matplotlib figure
            num_tracks = max(indices) + 1
            col = [colormap.jet(1.0 * c / (num_tracks - 1)) for c in indices]
            fig = plot(trackhits_x, trackhits_y, col)

            filename = 'cdchits_%i.png' % (self.num_events)
            if os.path.lexists(filename):
                B2WARNING(filename + ' exists, overwriting ...')
            else:
                B2INFO('creating ' + filename + ' ...')
            fig.savefig(filename)

        self.num_events += 1

    def terminate(self):
        """reimplementation of Module::terminate()."""
        B2INFO('terminating CDCPlotModule')


# Normal steering file part begins here

# Create main path
main = create_path()

evtmetagen = register_module('EvtMetaGen')

evtmetagen.param('EvtNumList', [5])

evtmetainfo = register_module('EvtMetaInfo')

gearbox = register_module('Gearbox')
geo = register_module('Geometry')
# Outer detectors are disabled for performance reasons.
# Note that this may produce a larger number of particles reentering
# the detector from the outside.
geo.param('ExcludedComponents', ['TOP', 'ECL', 'BKLM', 'EKLM'])

# particle gun to shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate
param_pGun = {
    'pdgCodes': [211, -211],
    'nTracks': 4,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.4, 1.6],
    'thetaGeneration': 'uniform',
    'thetaParams': [60., 120.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

plotmodule = CDCPlotModule()

main.add_module(evtmetagen)
main.add_module(gearbox)
main.add_module(geo)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(plotmodule)

process(main)
print statistics
