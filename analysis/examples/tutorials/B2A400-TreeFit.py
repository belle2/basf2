#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
# run 'pydoc3 vertex' for general documentation of vertexxing stuff
#
# I recommend to use the TreeFitter for everything as it is the fastest tool
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
#
# Note: This example is build upon
# B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.py
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: J.F.Krohn
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import massKFit
from stdPi0s import stdPi0s

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = ['/group/belle2/tutorial/release_01-00-00/mdst-B0D0pi0.root']
out_put_name = 'mdst-B0D0pi0-TreeFitted.root'

inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi0:looseFit" ParticleList
stdPi0s('looseFit')

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
reconstructDecay('D0:pi0pi0 -> pi0:looseFit pi0:looseFit', '1.7 < M < 2.0')


# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
reconstructDecay('B0:all -> D0:pi0pi0 pi0:looseFit', '5.24 < Mbc < 5.29 and abs(deltaE) < 1.0')

vertexTree(list_name='B0:all',
           conf_level=-1,  # keep all cadidates, 0:keep only fit survivors, optimise this cut for your need
           ipConstraint=True,
           # pins the B0 PRODUCTION vertex to the IP (increases SIG and BKG rejection) use for better vertex resolution
           updateAllDaughters=True,  # update momenta off ALL particles
           massConstraint=[111]  # mass constrain ALL pi0
           )

# perform MC matching (MC truth association)
matchMCTruth('B0:all')

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
analysis_main.add_module('VariablesToNtuple',
                         treeName='B0',
                         particleList='B0:all',
                         variables=variables,
                         fileName=out_put_name)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
