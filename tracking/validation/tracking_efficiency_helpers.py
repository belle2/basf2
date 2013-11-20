#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    function to simulate 10 charged muon tracks with           #
#    fixed transverse momentum using the ParticleGun            #
#                                                               #
#    written by Michael Ziegler, KIT                            #
#    michael.ziegler2@kit.edu                                   #
#                                                               #
#################################################################

from simulation import add_simulation
from basf2 import *
import glob
import os


def run_simulation(path, pt_value, output_filename):
    '''Add needed modules to the path and set parameters and start it'''

    # components which are used for the simulation and reconstruction
    # In order to avoid, that tracks came back in the cdc after they have left
    # TOP is added in the simulation
    components = [
        'MagneticField',
        'BeamPipe',
        'PXD',
        'SVD',
        'CDC',
        'TOP',
        ]

    # evt meta
    eventinfosetter = register_module('EventInfoSetter')

    # generate one event
    eventinfosetter.param('expList', [1])
    eventinfosetter.param('runList', [1])
    eventinfosetter.param('evtNumList', [500])

    path.add_module(eventinfosetter)

    progress = register_module('Progress')
    path.add_module(progress)

    # ParticleGun
    pGun = register_module('ParticleGun')
    # choose the particles you want to simulate
    param_pGun = {
        'pdgCodes': [13, -13],
        'nTracks': 10,
        'varyNTracks': 0,
        'momentumGeneration': 'uniformPt',
        'momentumParams': [pt_value, pt_value],
        'vertexGeneration': 'fixed',
        'xVertexParams': [0.0],
        'yVertexParams': [0.0],
        'zVertexParams': [0.0],
        }
    pGun.param(param_pGun)

    path.add_module(pGun)

    background_files = []
    if os.environ.has_key('BELLE2_BACKGROUND_DIR'):
        background_files += glob.glob(os.environ['BELLE2_BACKGROUND_DIR']
                                      + '/PXD*.root')
        background_files += glob.glob(os.environ['BELLE2_BACKGROUND_DIR']
                                      + '/SVD*.root')
        background_files += glob.glob(os.environ['BELLE2_BACKGROUND_DIR']
                                      + '/CDC*.root')

        print 'Number of used background files (%d): ' % len(background_files)

    # add simulation modules to the path
    add_simulation(path, components, background_files)

    # write output root file
    root_output = register_module('RootOutput')
    root_output.param('outputFileName', output_filename)
    path.add_module(root_output)


