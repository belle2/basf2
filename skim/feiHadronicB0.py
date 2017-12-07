#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI.
#
#
#
#####################################################
import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
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

# apply some very loose cuts to reduce the number
# of Btag candidates
applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 ')


B0hadronicList = ['B0:generic']


skimOutputUdst('feiHadronicB0', B0hadronicList)
summaryOfLists(B0hadronicList)


process(analysis_main)

# print out the summary
print(statistics)
