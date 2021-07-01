#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 ee -> mu+mu- events.
#
# Usage: basf2 B2A111-SimulateMuonPairs.py
#
# Input: None
# Output: B2A111-SimulateMuonPairs.root
# Contributors: U.Tamponi (October 2019)
#
# Example steering file - 2019 Belle II Collaboration
#############################################################

import basf2 as b2
import simulation as si
import reconstruction as re
import beamparameters as bp

# create path
main = b2.create_path()

# Setting up number of events to generate
main.add_module('EventInfoSetter', evtNumList=[10])

beamparameters = bp.add_beamparameters(main, "Y4S")
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
si.add_simulation(path=main)

# reconstruction
re.add_reconstruction(path=main)

# mdst output
re.add_mdst_output(
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
