#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# This steering file generates, simulates, and reconstructs              #
# a sample of 10 ee -> mu+mu- events.                                    #
#                                                                        #
# Usage: basf2 B2A111-SimulateMuonPairs.py                               #
#                                                                        #
# Input: None                                                            #
# Output: B2A111-SimulateMuonPairs.root                                  #
#                                                                        #
##########################################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters
from mdst import add_mdst_output

# create path
main = b2.create_path()

# Setting up number of events to generate
main.add_module('EventInfoSetter', evtNumList=[10])

beamparameters = add_beamparameters(main, "Y4S")
beamparameters.param("smearVertex", False)
beamparameters.param("smearEnergy", False)
b2.print_params(beamparameters)

# muon pair generator (BABAYAGA.NLO, born only)
main.add_module('BabayagaNLOInput',
                FinalState='mm',
                SearchMax=10000,
                VacuumPolarization='hlmnt',
                Order='born',
                MaxAcollinearity=180.,
                DebugEnergySpread=5.e-3,
                ScatteringAngleRange=[15.0, 165.0],
                VPUncertainty=True)

# detector simulation
add_simulation(path=main)

# reconstruction
add_reconstruction(path=main)

# mdst output
add_mdst_output(
    path=main,
    mc=True,
    filename='B2A111-SimulateMuonPairs.root',
    additionalBranches=[
        'MCInitialParticles'])

# display a progress bar while running
main.add_module('ProgressBar')

# process events and print call statistics
b2.process(main)
print(b2.statistics)
