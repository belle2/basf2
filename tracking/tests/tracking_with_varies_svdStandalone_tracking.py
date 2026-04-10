#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import b2test_utils as b2tu
from basf2 import set_random_seed, create_path, process
from simulation import add_simulation
from tracking import add_tracking_reconstruction


def run_simulation(output_file):
    """Main function to be executed if this script"""
    set_random_seed(12345)
    main = create_path()

    # specify number of events to be generated
    main.add_module('EventInfoSetter', expList=[0], evtNumList=[5], runList=[1])
    main.add_module('Progress')
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
    main.add_module('RootOutput', outputFileName=output_file)
    process(main)


def run_tracking(input_file, mode):
    set_random_seed(12345)
    main = create_path()
    main.add_module('RootInput', inputFileName=input_file)

    main.add_module('Gearbox')
    main.add_module('Geometry')
    add_tracking_reconstruction(main, pruneTracks=False, svd_standalone_mode=mode)
    process(main)


if __name__ == "__main__":

    with b2tu.clean_working_directory():
        dst_file = 'dst.root'
        b2tu.run_in_subprocess(target=run_simulation, output_file=dst_file)
        for mode in ['VXDTF2', 'SVDHough', 'VXDTF2_and_SVDHough', 'SVDHough_and_VXDTF2']:
            b2tu.run_in_subprocess(target=run_tracking, input_file=dst_file, mode=mode)
