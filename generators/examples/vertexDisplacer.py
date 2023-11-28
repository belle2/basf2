#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output
import modularAnalysis as ma

main = b2.create_path()

eventinfosetter = main.add_module("EventInfoSetter", evtNumList=[100], expList=[1003])

# Generate decay with your favourite generator, using EvtGen as example
# the decay file should include the particle(s) to be displaced
dec_file = None
if len(sys.argv) > 1:
    dec_file = os.path.abspath(sys.argv[1])
    print("using following decay file: " + dec_file)

main.add_module('EvtGenInput', ParentParticle='Upsilon(4S)', userDECFile=dec_file)

# initialise the displacer module
displacer = b2.register_module("GeneratedVertexDisplacer")

# list of pdg values for particles to be displaced, here 100
displacer.param("pdgVal", (100))

# list of lifetimes corresponding to the particles to be displaced, here c*tau = 10cm
displacer.param("lifetime", (10))
# pass the lifetime in units of [ns]: displacer.param("ctau", False)

# specifying exponential decay option
displacer.param("lifetimeOption", "exponential")

# add displacer module between generator and detector simulation
main.add_module(displacer)

# print the MC particles for verification
ma.printMCParticles(onlyPrimaries=False, maxLevel=-1, path=main)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

# save to file
add_mdst_output(main, filename='displacerOut.root')

b2.process(main)
