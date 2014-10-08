#!/usr/bin/env python
# -*- coding: utf-8 -*-

# !/usr/bin/env python

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
import sys


def get_generated_pt_value(index):
    """
    List with generated pt values is created in this function. With the
    parameter index, one is able to access single pt values. If index == -1,
    the number of pt values in the list is returned.

    @param index

    @return double
    """

    # pt_values = [0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 2.5, 3.0]
    pt_values = [
        0.1,
        0.25,
        0.4,
        0.6,
        1.,
        1.5,
        2.,
        3.,
        4.,
        ]

    if index == -1:
        return len(pt_values)
    try:
        return pt_values[index]
    except IndexError:
        print 'ERROR in %s. Index is out of range. Only %d elements in list.' \
            % (get_generated_pt_value.__name__, len(pt_values))
        sys.exit(1)


def get_generated_pt_values():
    """
    Collects all pt values with help of the function get_generated_pt_value
    and stores them in a list, that is returned

    @return list of pt values
    """

    list = []
    for index in xrange(get_generated_pt_value(-1)):
        list.append(get_generated_pt_value(index))
    return list


def run_simulation(path, pt_value, output_filename):
    """Add needed modules to the path and set parameters and start it"""

    # components which are used for the simulation and reconstruction
    # In order to avoid, that tracks came back in the cdc after they have left
    # TOP is added in the simulation
    components = ['MagneticFieldConstant4LimitedRCDC', 'BeamPipe', 'PXD', 'SVD'
                  , 'CDC']

    # evt meta
    eventinfosetter = register_module('EventInfoSetter')

    # generate one event
    eventinfosetter.param('expList', [1])
    eventinfosetter.param('runList', [1])
    eventinfosetter.param('evtNumList', [200])

    path.add_module(eventinfosetter)

    progress = register_module('Progress')
    path.add_module(progress)

    # ParticleGun
    pgun = register_module('ParticleGun')
    # choose the particles you want to simulate
    param_pgun = {
        'pdgCodes': [13, -13],
        'nTracks': 10,
        'varyNTracks': 0,
        'momentumGeneration': 'uniformPt',
        'momentumParams': [pt_value, pt_value],
        'vertexGeneration': 'fixed',
        'xVertexParams': [0.0],
        'yVertexParams': [0.0],
        'zVertexParams': [0.0],
        'thetaGeneration': 'uniformCos',
        }

    pgun.param(param_pgun)

    path.add_module(pgun)

    background_files = []
    if 'BELLE2_BACKGROUND_DIR' in os.environ:
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


