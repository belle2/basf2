#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

""" This script creates 10 events with 12 charged stable particles
    per event, then extracts CNN value for each particle.
    The CNN values represent probabilities for a track being
    muon or pion like.

INPUT:
    No input

USAGE:
    basf2 EclCNNPID.py -- [--beam-bkg-dir <path_to_beam_bkg_files>]

EXAMPLE:
    basf2 EclCNNPID.py

IMPORTANT NOTE:
    In this example script Gearbox and Geometry modules
    are automatically registered in add_simulation().

    In order to use CNN_PID_ECL module in your script,
    it is essential to add the following lines before
    fillParticleList() function:
        mainPath.add_module('Gearbox')
        mainPath.add_module('Geometry')
"""


import argparse


def argparser():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        '--beam-bkg-dir',
        type=str,
        default=None,
        help='Directory that contains beam background files'
        'If nothing is specified, basf2 will used by default'
        'BELLE2_BACKGROUND_DIR env variable.')

    return parser


if __name__ == '__main__':

    args = argparser().parse_args()

    import basf2 as b2  # noqa
    from ROOT import Belle2  # noqa
    import modularAnalysis as ma  # noqa
    from simulation import add_simulation  # noqa
    from reconstruction import add_reconstruction  # noqa
    from background import get_background_files  # noqa
    from eclCNNPID import CNN_PID_ECL  # noqa

    chargedStable_pdg_list = []
    for idx in range(len(Belle2.Const.chargedStableSet)):
        particle = Belle2.Const.chargedStableSet.at(idx)
        pdgId = particle.getPDGCode()
        chargedStable_pdg_list.extend([pdgId, -pdgId])

    # Necessary global tag for CNN_PID_ECL module
    b2.conditions.prepend_globaltag('cnn_pid_ecl')

    # Path
    mainPath = b2.create_path()

    b2.set_log_level(b2.LogLevel.WARNING)

    # Register ParticleGun module
    particleGun = b2.register_module('ParticleGun')
    b2.set_random_seed(123)
    particleGun_param = {
        'pdgCodes': chargedStable_pdg_list,
        'nTracks': 0,  # 0 means it generates 1 track per pdgId per event.
        'momentumGeneration': 'uniformPt',
        'momentumParams': [0.3, 0.9],
        'thetaGeneration': 'uniform',
        'thetaParams': [70, 90],  # In the ECL barrel
        'phiGeneration': 'uniform',
        'phiParams': [0, 360],
        'xVertexParams': [0.0, 0.0],
        'yVertexParams': [0.0, 0.0],
        'zVertexParams': [0.0, 0.0],
    }
    particleGun.param(particleGun_param)
    mainPath.add_module(particleGun)

    mainPath.add_module(
        'EventInfoSetter',
        expList=1003,  # Exp 1003 is early phase 3
        runList=0,
        evtNumList=10)

    if args.beam_bkg_dir:
        # Simulation with beam background
        add_simulation(
            mainPath,
            bkgfiles=get_background_files(
                folder=args.beam_bkg_dir
            )
        )
    else:
        # Simulation
        add_simulation(mainPath)

    # Reconstruction
    add_reconstruction(mainPath)

    name = 'pi+:particles'
    ma.fillParticleList(name, '', path=mainPath)

    # CNN_PID_ECL module
    mainPath.add_module(CNN_PID_ECL(particleList=name, path=mainPath))

    ma.variablesToNtuple(
        decayString=name,
        variables=['cnn_pid_ecl_pion', 'cnn_pid_ecl_muon'],
        treename='particles',
        filename=f'test_chargedStable_particles_cnn_output.root',
        path=mainPath
    )

    mainPath.add_module('Progress')
    b2.process(mainPath)
    print(b2.statistics)
