#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#################################################################
#                                                               #
#    function to simulate 10 charged muon tracks with           #
#    fixed transverse momentum using the ParticleGun            #
#                                                               #
#################################################################

from simulation import add_simulation
from reconstruction import add_reconstruction
import basf2 as b2
import glob
import os
import sys


def get_generated_pdg_code():
    return 13


def get_simulation_components():
    return None


def get_reconstruction_components():
    # Could be different from get_simulation_components, if only testing subdetectors.
    return get_simulation_components()


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
        0.05,
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
        print('ERROR in %s. Index is out of range. Only %d elements in list.'
              % (get_generated_pt_value.__name__, len(pt_values)))
        sys.exit(1)


def get_generated_pt_values():
    """
    Collects all pt values with help of the function get_generated_pt_value
    and stores them in a list, that is returned

    @return list of pt values
    """

    list = []
    for index in range(get_generated_pt_value(-1)):
        list.append(get_generated_pt_value(index))

    return list


def additional_options(path):
    """
    This sets the specifics of simulation and reconstruction in order
    to use consistent settings across the various involved modules.
    """

    realisticCDCGeo = 1
    useInWirePropagation = 1
    useTime = 1
    useWireSag = 1
    for m in path.modules():
        if realisticCDCGeo:
            if m.type() == 'CDCDigitizer':
                m.param('AddInWirePropagationDelay', useInWirePropagation)
                m.param('AddTimeOfFlight', useTime)
                m.param('CorrectForWireSag', useWireSag)
        else:
            if m.type() == 'CDCDigitizer':
                m.param('AddInWirePropagationDelay', 0)
                m.param('AddTimeOfFlight', 0)
                m.param('CorrectForWireSag', 0)

        if m.type() == 'DAFRecoFitter':
            m.param('pdgCodesToUseForFitting', [get_generated_pdg_code()])

        if m.type() == "TrackCreator":
            m.param('pdgCodes', [get_generated_pdg_code(), 211])


def run_simulation(path, pt_value, output_filename=''):
    """Add needed modules to the path and set parameters and start it"""

    # evt meta
    eventinfosetter = b2.register_module('EventInfoSetter')

    # generate one event
    eventinfosetter.param('expList', [0])
    eventinfosetter.param('runList', [1])
    eventinfosetter.param('evtNumList', [200])

    path.add_module(eventinfosetter)

    progress = b2.register_module('Progress')
    path.add_module(progress)

    # ParticleGun
    pgun = b2.register_module('ParticleGun')
    # choose the particles you want to simulate
    param_pgun = {
        'pdgCodes': [get_generated_pdg_code(), -get_generated_pdg_code()],
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
        background_files += glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')

        print(f'Number of used background files ({len(background_files)}): ')

    if len(background_files) == 0:
        background_files = None

    # add simulation modules to the path
    add_simulation(path, get_simulation_components(), background_files)

    additional_options(path)
    # write output root file
    if output_filename != '':
        root_output = b2.register_module('RootOutput')
        root_output.param('outputFileName', output_filename)
        path.add_module(root_output)


def run_reconstruction(path, output_file_name, input_file_name=''):
    if input_file_name != '':
        root_input = b2.register_module('RootInput')
        root_input.param('inputFileNames', input_file_name)
        path.add_module(root_input)

        gearbox = b2.register_module('Gearbox')
        path.add_module(gearbox)

        geometry = b2.register_module('Geometry')
        geometry.param("useDB", True)
        path.add_module(geometry)

    add_reconstruction(path, get_reconstruction_components(), pruneTracks=0)
    # add_mc_reconstruction(path, get_reconstruction_components(), pruneTracks=0)

    tracking_efficiency = b2.register_module('StandardTrackingPerformance')
    # tracking_efficiency.logging.log_level = LogLevel.DEBUG
    tracking_efficiency.param('outputFileName', output_file_name)
    path.add_module(tracking_efficiency)

    additional_options(path)
