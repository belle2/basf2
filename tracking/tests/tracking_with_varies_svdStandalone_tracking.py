#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from basf2 import set_random_seed, create_path, process
from simulation import add_simulation
from tracking import add_tracking_reconstruction
import logging


def main():
    """Main function to be executed if this script"""
    set_random_seed(12345)

    main = create_path()

    # specify number of events to be generated
    main.add_module('EventInfoSetter', expList=[0], evtNumList=[5], runList=[1])
    main.add_module('ParticleGun',
                    pdgCodes=[211],
                    nTracks=1,
                    momentumGeneration='fixed',
                    momentumParams=[1.618034],
                    phiGeneration='fixed',
                    phiParams=[27.182818],
                    thetaGeneration='fixed',
                    thetaParams=[62.83185])
    add_simulation(main, bkgfiles=None)
    add_tracking_reconstruction(main, pruneTracks=False, svd_standalone_mode='VXDTF2_and_SVDHough')
    # add_tracking_reconstruction(main, pruneTracks=False, svd_standalone_mode='SVDHough_and_VXDTF2')
    # add_tracking_reconstruction(main, pruneTracks=False, svd_standalone_mode='VXDTF2')
    # add_tracking_reconstruction(main, pruneTracks=False, svd_standalone_mode='SVDHough')

    main.add_module('Progress')
    process(main)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    main()
