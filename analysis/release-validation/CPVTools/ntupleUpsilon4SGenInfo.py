#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>Vladimir Chekelian; Fernando Abudinen; abudinen@mpp.mpg.de</contact>
  <description> make Upsilon(4S) ntuple for checks and studies of the generator file:
particles: Upsilon(4S), two daughters (i.e. BBbar) and four granddaughters (e.g. B0->Jpsi+K0S)
variables: mcPDG,M,nDaughters,mcDecayTime,mcLifeTime,mcX,mcY,mcZ,mcE,mcPX,mcPY,mcPZ;
created 12.02.2019 </description>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from variables import variables
import sys

variables.addAlias('B00_mcPDG', 'daughter(0, mcPDG)')
variables.addAlias('B00_M', 'daughter(0, M)')
variables.addAlias('B00_nDaughters', 'daughter(0, nDaughters)')
variables.addAlias('B00_mcDecayTime', 'daughter(0, mcDecayTime)')
variables.addAlias('B00_mcLifeTime', 'daughter(0, mcLifeTime)')
variables.addAlias('B00_mcX', 'daughter(0, mcDecayVertexX)')
variables.addAlias('B00_mcY', 'daughter(0, mcDecayVertexY)')
variables.addAlias('B00_mcZ', 'daughter(0, mcDecayVertexZ)')
variables.addAlias('B00_mcE', 'daughter(0, mcE)')
variables.addAlias('B00_mcPX', 'daughter(0, mcPX)')
variables.addAlias('B00_mcPY', 'daughter(0, mcPY)')
variables.addAlias('B00_mcPZ', 'daughter(0, mcPZ)')

variables.addAlias('B00_daughter0_mcPDG', 'daughter(0, daughter(0, mcPDG))')
variables.addAlias('B00_daughter0_M', 'daughter(0, daughter(0, M))')
variables.addAlias('B00_daughter0_nDaughters', 'daughter(0, daughter(0, nDaughters))')
variables.addAlias('B00_daughter0_mcDecayTime', 'daughter(0, daughter(0, mcDecayTime))')
variables.addAlias('B00_daughter0_mcLifeTime', 'daughter(0, daughter(0, mcLifeTime))')
variables.addAlias('B00_daughter0_mcX', 'daughter(0, daughter(0, mcDecayVertexX))')
variables.addAlias('B00_daughter0_mcY', 'daughter(0, daughter(0, mcDecayVertexY))')
variables.addAlias('B00_daughter0_mcZ', 'daughter(0, daughter(0, mcDecayVertexZ))')
variables.addAlias('B00_daughter0_mcE', 'daughter(0, daughter(0, mcE))')
variables.addAlias('B00_daughter0_mcPX', 'daughter(0, daughter(0, mcPX))')
variables.addAlias('B00_daughter0_mcPY', 'daughter(0, daughter(0, mcPY))')
variables.addAlias('B00_daughter0_mcPZ', 'daughter(0, daughter(0, mcPZ))')

variables.addAlias('B00_daughter1_mcPDG', 'daughter(0, daughter(1, mcPDG))')
variables.addAlias('B00_daughter1_M', 'daughter(0, daughter(1, M))')
variables.addAlias('B00_daughter1_nDaughters', 'daughter(0, daughter(1, nDaughters))')
variables.addAlias('B00_daughter1_mcDecayTime', 'daughter(0, daughter(1, mcDecayTime))')
variables.addAlias('B00_daughter1_mcLifeTime', 'daughter(0, daughter(1, mcLifeTime))')
variables.addAlias('B00_daughter1_mcX', 'daughter(0, daughter(1, mcDecayVertexX))')
variables.addAlias('B00_daughter1_mcY', 'daughter(0, daughter(1, mcDecayVertexY))')
variables.addAlias('B00_daughter1_mcZ', 'daughter(0, daughter(1, mcDecayVertexZ))')
variables.addAlias('B00_daughter1_mcE', 'daughter(0, daughter(1, mcE))')
variables.addAlias('B00_daughter1_mcPX', 'daughter(0, daughter(1, mcPX))')
variables.addAlias('B00_daughter1_mcPY', 'daughter(0, daughter(1, mcPY))')
variables.addAlias('B00_daughter1_mcPZ', 'daughter(0, daughter(1, mcPZ))')

variables.addAlias('B01_mcPDG', 'daughter(1, mcPDG)')
variables.addAlias('B01_M', 'daughter(1, M)')
variables.addAlias('B01_nDaughters', 'daughter(1, nDaughters)')
variables.addAlias('B01_mcDecayTime', 'daughter(1, mcDecayTime)')
variables.addAlias('B01_mcLifeTime', 'daughter(1, mcLifeTime)')
variables.addAlias('B01_mcX', 'daughter(1, mcDecayVertexX)')
variables.addAlias('B01_mcY', 'daughter(1, mcDecayVertexY)')
variables.addAlias('B01_mcZ', 'daughter(1, mcDecayVertexZ)')
variables.addAlias('B01_mcE', 'daughter(1, mcE)')
variables.addAlias('B01_mcPX', 'daughter(1, mcPX)')
variables.addAlias('B01_mcPY', 'daughter(1, mcPY)')
variables.addAlias('B01_mcPZ', 'daughter(1, mcPZ)')

variables.addAlias('B01_daughter0_mcPDG', 'daughter(1, daughter(0, mcPDG))')
variables.addAlias('B01_daughter0_M', 'daughter(1, daughter(0, M))')
variables.addAlias('B01_daughter0_nDaughters', 'daughter(1, daughter(0, nDaughters))')
variables.addAlias('B01_daughter0_mcDecayTime', 'daughter(1, daughter(0, mcDecayTime))')
variables.addAlias('B01_daughter0_mcLifeTime', 'daughter(1, daughter(0, mcLifeTime))')
variables.addAlias('B01_daughter0_mcX', 'daughter(1, daughter(0, mcDecayVertexX))')
variables.addAlias('B01_daughter0_mcY', 'daughter(1, daughter(0, mcDecayVertexY))')
variables.addAlias('B01_daughter0_mcZ', 'daughter(1, daughter(0, mcDecayVertexZ))')
variables.addAlias('B01_daughter0_mcE', 'daughter(1, daughter(0, mcE))')
variables.addAlias('B01_daughter0_mcPX', 'daughter(1, daughter(0, mcPX))')
variables.addAlias('B01_daughter0_mcPY', 'daughter(1, daughter(0, mcPY))')
variables.addAlias('B01_daughter0_mcPZ', 'daughter(1, daughter(0, mcPZ))')

variables.addAlias('B01_daughter1_mcPDG', 'daughter(1, daughter(1, mcPDG))')
variables.addAlias('B01_daughter1_M', 'daughter(1, daughter(1, M))')
variables.addAlias('B01_daughter1_nDaughters', 'daughter(1, daughter(1, nDaughters))')
variables.addAlias('B01_daughter1_mcDecayTime', 'daughter(1, daughter(1, mcDecayTime))')
variables.addAlias('B01_daughter1_mcLifeTime', 'daughter(1, daughter(1, mcLifeTime))')
variables.addAlias('B01_daughter1_mcX', 'daughter(1, daughter(1, mcDecayVertexX))')
variables.addAlias('B01_daughter1_mcY', 'daughter(1, daughter(1, mcDecayVertexY))')
variables.addAlias('B01_daughter1_mcZ', 'daughter(1, daughter(1, mcDecayVertexZ))')
variables.addAlias('B01_daughter1_mcE', 'daughter(1, daughter(1, mcE))')
variables.addAlias('B01_daughter1_mcPX', 'daughter(1, daughter(1, mcPX))')
variables.addAlias('B01_daughter1_mcPY', 'daughter(1, daughter(1, mcPY))')
variables.addAlias('B01_daughter1_mcPZ', 'daughter(1, daughter(1, mcPZ))')

if len(sys.argv) != 2:
    sys.exit('Must provide one input parameter: [output_root_file_name]'
             )

outRootFileName = sys.argv[1]

# create path
cp_val_path = b2.create_path()

environmentType = "default"

ma.inputMdstList(environmentType=environmentType, filelist=[], path=cp_val_path)

Upsilon4S = ('Upsilon(4S):all', '')
ma.fillParticleListsFromMC([Upsilon4S], True, path=cp_val_path)
ma.matchMCTruth('Upsilon(4S):all', path=cp_val_path)

ma.variablesToNtuple(decayString='Upsilon(4S):all',
                     variables=["generatorEventWeight",
                                # Upsilon(4S)
                                "mcPDG", "M", "nDaughters", "mcDecayTime", "mcLifeTime",
                                "mcX", "mcY", "mcZ", "mcE", "mcPX", "mcPY", "mcPZ",
                                # first daughter (e.g. 1st B0) and two granddaughters
                                'B00_mcPDG', 'B00_M', 'B00_nDaughters',
                                'B00_mcDecayTime', 'B00_mcLifeTime',
                                'B00_mcX', 'B00_mcY', 'B00_mcZ',
                                'B00_mcE', 'B00_mcPX', 'B00_mcPY', 'B00_mcPZ',
                                'B00_daughter0_mcPDG', 'B00_daughter1_mcPDG',
                                'B00_daughter0_M', 'B00_daughter1_M',
                                'B00_daughter0_mcDecayTime', 'B00_daughter1_mcDecayTime',
                                'B00_daughter0_mcLifeTime', 'B00_daughter1_mcLifeTime',
                                'B00_daughter0_mcX', 'B00_daughter1_mcX',
                                'B00_daughter0_mcY', 'B00_daughter1_mcY',
                                'B00_daughter0_mcZ', 'B00_daughter1_mcZ',
                                'B00_daughter0_mcE', 'B00_daughter1_mcE',
                                'B00_daughter0_mcPX', 'B00_daughter1_mcPX',
                                'B00_daughter0_mcPY', 'B00_daughter1_mcPY',
                                'B00_daughter0_mcPZ', 'B00_daughter1_mcPZ',
                                # second daughter (e.g. 2nd B0) and two granddaughters
                                'B01_mcPDG', 'B01_M', 'B01_nDaughters',
                                'B01_mcDecayTime', 'B01_mcLifeTime',
                                'B01_mcX', 'B01_mcY', 'B01_mcZ',
                                'B01_mcE', 'B01_mcPX', 'B01_mcPY', 'B01_mcPZ',
                                'B01_daughter0_mcPDG', 'B01_daughter1_mcPDG',
                                'B01_daughter0_M', 'B01_daughter1_M',
                                'B01_daughter0_mcDecayTime', 'B01_daughter1_mcDecayTime',
                                'B01_daughter0_mcLifeTime', 'B01_daughter1_mcLifeTime',
                                'B01_daughter0_mcX', 'B01_daughter1_mcX',
                                'B01_daughter0_mcY', 'B01_daughter1_mcY',
                                'B01_daughter0_mcZ', 'B01_daughter1_mcZ',
                                'B01_daughter0_mcE', 'B01_daughter1_mcE',
                                'B01_daughter0_mcPX', 'B01_daughter1_mcPX',
                                'B01_daughter0_mcPY', 'B01_daughter1_mcPY',
                                'B01_daughter0_mcPZ', 'B01_daughter1_mcPZ'],
                     filename=outRootFileName,
                     treename='UpsilonTree',
                     path=cp_val_path)

ma.summaryOfLists(particleLists=['Upsilon(4S):all'], path=cp_val_path)

# Process the events
ma.process(cp_val_path)

# print out the summary
print(b2.statistics)
