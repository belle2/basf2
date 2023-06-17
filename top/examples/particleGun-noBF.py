#!/usr/bin/env python3

# -------------------------------------------------------------------------------------------------------
# Example on how to simulate/reconstruct single particle events w/o intoducing bunch finder inefficiency.
# Could be useful for AI training.
#
# usage: basf2 particleGunSimu-noBF.py -o <my_output_filename.root> -n <events_to_generate>
# -------------------------------------------------------------------------------------------------------

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output

main = b2.create_path()
main.add_module('EventInfoSetter')
main.add_module('ParticleGun',
                pdgCodes=[321, -321],
                nTracks=1,
                varyNTracks=False,  # will generate single kaon per event
                momentumGeneration='uniform',
                momentumParams=[0.05, 5.0],
                thetaGeneration='uniformCos',  # this makes angular distribution uniform in the solid angle
                phiGeneration='uniform')
add_simulation(main, simulateT0jitter=False)  # switch-off simulation of L1 trigger time jitter
add_reconstruction(main)
main = b2.remove_module(main, 'TOPBunchFinder')  # remove bunch finder from the path

# output to file: use -o option to specify file name
add_mdst_output(main)

main.add_module('Progress')
b2.process(main)

# Print call statistics
print(b2.statistics)
