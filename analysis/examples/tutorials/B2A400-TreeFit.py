#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# run 'pydoc3 vertex' for general documentation of vertexxing stuff
#
# It is recommended to use the TreeFitter for everything as it is the fastest tool
#
# If you want to fit precise vertices with nTracks>2 attached and you are
# interested in the vertex postion, TagV COULD BE the better tool as it
# reweights the tracks. However, you have to check yourself.
#
# This tutorial demonstrates how to perform a fit with
# the TreeFitter. In this example the following decay chain:
#
# B0 -> D0 pi0
#       |
#       +-> pi0 pi0
#
# is reconstructed.
#
# Note: This example is build upon
# B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.py
#
# Contributors: J.F.Krohn (2018)
#               I. Komarov (September 2018)
#
######################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import vertex as vx
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)


# use standard final state particle lists
#
# creates "pi0:looseFit" ParticleList
# https://confluence.desy.de/display/BI/Physics+StandardParticles
stdPi0s(listtype='looseFit', path=my_path)

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
ma.reconstructDecay(decayString='D0:pi0pi0 -> pi0:looseFit pi0:looseFit',
                    cut='1.7 < M < 2.0',
                    path=my_path)

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
ma.reconstructDecay(decayString='B0:all -> D0:pi0pi0 pi0:looseFit',
                    cut='5.24 < Mbc < 5.29 and abs(deltaE) < 1.0',
                    path=my_path)

vx.vertexTree(list_name='B0:all',
              conf_level=-1,  # keep all cadidates, 0:keep only fit survivors, optimise this cut for your need
              ipConstraint=True,
              # pins the B0 PRODUCTION vertex to the IP (increases SIG and BKG rejection) use for better vertex resolution
              updateAllDaughters=True,  # update momenta off ALL particles
              massConstraint=[111],  # mass constrain ALL pi0
              path=my_path
              )

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='B0:all',
                path=my_path)

# whatever you are interested in
#
# see analysis/VariableManager/ for implementation of the vars
variables = [
    'isSignal',  # MC truth
    'chiProb',  # pValue of the fit use this in your analysis to reject background
    "p",  # momentum (of the B0)
    "mcP",  # generated momentum
    "pErr",  # momentum uncertainty taking the full px, py, pz covariance into account
]

# safe the output
output_name = "B2A400-TreeFit.root"
my_path.add_module('VariablesToNtuple',
                   treeName='B0',
                   particleList='B0:all',
                   variables=variables,
                   fileName=output_name)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
