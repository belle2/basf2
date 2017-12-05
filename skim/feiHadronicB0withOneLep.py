#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script demonstrates how to reconstruct hadronic  Btag using
# generically trained FEI with Bsig to  at least one lepton.
#
# Racha Cheaib (2017)
#
#####################################################
import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from stdCharged import *
gb2_setuprel = 'build-2017-09-08'
reset_database()
use_local_database('/cvmfs/belle.cern.ch/conditions/GT_gen_prod_003.01_Master-20170721-132500-FEI-skim-a.txt', readonly=True)


fileList =\
    ['/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000273/s00/e0000/4S/r00000/signal/sub00/*'
     ]
inputMdstList('default', fileList)


import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)

# now the FEI reconstruction is done
# and we're back in analysis_main pathB

from variables import variables
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
# apply some very loose cuts to reduce the number
# of Btag candidates
applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200')

# now signal side
stdMu('all')
stdE('all')


reconstructDecay('B0:sig1 -> e+:all', 'Mbc>0', 1)
reconstructDecay('B0:sig2 -> mu+:all', 'Mbc>0', 2)
reconstructDecay('B0:sig3 -> e-:all', 'Mbc>0', 3)
reconstructDecay('B0:sig4 -> mu-:all', 'Mbc>0', 4)

copyLists('B0:all', ['B0:sig1', 'B0:sig2', 'B0:sig3', 'B0:sig4'])

reconstructDecay('Upsilon(4S):sig -> anti-B0:generic B0:all', '')

buildRestOfEvent('Upsilon(4S):sig')

UpsilonList = ['Upsilon(4S):sig']

skimOutputUdst('feiHadronicB0withOneLep', UpsilonList)
summaryOfLists(UpsilonList)


process(analysis_main)

# print out the summary
print(statistics)
