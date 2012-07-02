#!/usr/bin/env python
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
from matplotlib.patches import Circle


def plot(x, y, show=0):
    fig = plt.figure(figsize=(8, 8))
    fig.subplots_adjust(bottom=0.1, left=0.1, right=0.95, top=0.95)
    ax = fig.add_subplot(111)

    # draw the x/y arrays. note that looping over the hits and
    # drawing them individually would be much slower
    ax.plot(
        x,
        y,
        marker='.',
        color='blue',
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


from basf2 import *

from ROOT import Belle2

import os


class CDCPlotModule(Module):

    num_events = 0
    trackhits_x = []
    trackhits_y = []

    def __init__(self):
        super(CDCPlotModule, self).__init__()
        self.setName('CDCPlotModule')

    def event(self):
        store = Belle2.PyStoreArray('CDCSimHits')
        entries = store.getEntries()
        for i in range(entries):
            hitpos = store[i].m_posWire  # TVector3
            self.trackhits_x.append(hitpos.x())
            self.trackhits_y.append(hitpos.y())
        self.num_events += 1

    def terminate(self):
        B2INFO('terminating CDCPlotModule')
        # plot the (x,y) list on a matplotlib figure
        fig = plot(self.trackhits_x, self.trackhits_y)
        if os.path.lexists('hits_all.png'):
            B2WARNING('hits_all.png exists, overwriting ...')
        else:
            B2INFO('creating hits_all.png ...')
        fig.savefig('hits_all')


# Normal steering file part begins here

import random

# Create main path
main = create_path()

evtmetagen = register_module('EvtMetaGen')

evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
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
