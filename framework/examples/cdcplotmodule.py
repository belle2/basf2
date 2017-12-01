#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################################
# A more complex python module using matlotlib to create
# advanced plots.
# It gathers the x/y position off all CDCSimHits and draws them in different
# colours depending on associated MCParticle.
###############################################################################

from basf2 import *
from simulation import add_simulation

import matplotlib
# to avoid loading gtk backend in the following imports
# (fails if no X11 display is available)
matplotlib.use('agg')

import matplotlib.pyplot as plt
from matplotlib.patches import Circle
import matplotlib.cm as colormap

from ROOT import Belle2
import os


def plot(x, y, col, show=0):
    """
    Plot a list of x/y values, plus CDC superlayer boundaries.

    Returns a pyplot.figure that can be saved.
    """
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111)

    # draw the x/y arrays. note that looping over the hits and
    # drawing them individually would be much slower
    for i in range(len(col)):
        ax.plot(x[i], y[i], marker='.', color=col[i], linestyle='None', markersize=1)

    ax.set_title('CDCSimHits')
    ax.set_xlabel('x [cm]')
    ax.set_ylabel('y [cm]')
    ax.axis('scaled')

    # draw CDC superlayer boundaries
    layers = [16.8, 25.7, 36.5, 47.6, 58.4, 69.5, 80.2, 91.3, 102.0, 111.1]
    Circs = [Circle((0, 0), a, facecolor='none', edgecolor='lightgrey')
             for a in layers]
    for e in Circs:
        ax.add_artist(e)

    ax.set_xlim(-130, 130)
    ax.set_ylim(-130, 130)
    fig.tight_layout()
    if show:
        plt.show()
    return fig


class CDCPlotModule(Module):
    """An example python module.

    It gathers the x/y position off all CDCSimHits and draws them using
    matplotlib. The result is saved as a PNG.
    """

    #: event counter
    num_events = 0

    def event(self):
        """reimplementation of Module::event().

        loops over the CDCSimHits in the current event.
        """
        simhits = Belle2.PyStoreArray('CDCSimHits')

        # list of lists of simhit positions, one list per mcpart
        trackhits_x = []
        trackhits_y = []

        mcparts = []
        for hit in simhits:
            mcpart = hit.getRelatedFrom("MCParticles")
            if mcpart not in mcparts:
                mcparts.append(mcpart)
                trackhits_x.append([])
                trackhits_y.append([])
            # add simhit to the list corresponding to this particle
            idx = mcparts.index(mcpart)
            hitpos = hit.getPosWire()  # TVector3
            trackhits_x[idx].append(hitpos.x())
            trackhits_y[idx].append(hitpos.y())

        npart = len(mcparts)
        if npart > 0:
            # plot the (x,y) list on a matplotlib figure
            col = [colormap.jet(1.0 * c / (npart - 1)) for c in range(npart)]
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

# Create main path
main = create_path()
main.add_module('EventInfoSetter', evtNumList=[5])
main.add_module('ParticleGun', **param_pGun)
add_simulation(main)
main.add_module(CDCPlotModule())

process(main)
print(statistics)
