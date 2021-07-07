#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#################################################################
#                                                               #
#    Script to simulate 2000 charged-muon single-track events   #
#    using the ParticleGun for BKLM and EKLM validation.        #
#                                                               #
#################################################################

"""
<header>
    <output>muon-KLMValidation.root</output>
    <contact>martina.laurenza@roma3.infn.it</contact>
    <description>Create events with 1 muon track for BKLM and EKLM validation.</description>
</header>
"""


import basf2
import simulation as sim
import reconstruction as rec

basf2.set_random_seed(981543)
basf2.set_log_level(basf2.LogLevel.ERROR)

output_filename = '../muon-KLMValidation.root'
basf2.B2INFO('The output file name is ' + output_filename)

main_path = basf2.create_path()

main_path.add_module('EventInfoSetter', evtNumList=2000)

main_path.add_module('Progress')
main_path.add_module('ProgressBar')

main_path.add_module('ParticleGun',
                     pdgCodes=[-13, 13],
                     nTracks=1,
                     varyNTracks=0,
                     momentumGeneration='uniform',
                     momentumParams=[0.5, 5.0],
                     thetaGeneration='uniformCos',
                     thetaParams=[18., 155.],
                     phiGeneration='uniform',
                     phiParams=[0., 360.],
                     vertexGeneration='fixed',
                     xVertexParams=[0.0],
                     yVertexParams=[0.0],
                     zVertexParams=[0.0])

sim.add_simulation(path=main_path)
rec.add_reconstruction(path=main_path)

main_path.add_module('RootOutput',
                     outputFileName=output_filename,
                     branchNames=['MCParticles', 'KLMMuidLikelihoods', 'KLMDigits', 'BKLMHit2ds', 'EKLMHit2ds'])

basf2.process(main_path)
print(basf2.statistics)
