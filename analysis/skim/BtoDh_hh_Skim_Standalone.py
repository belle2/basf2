#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Hulya Atmacan 2016/Oct/25
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *

set_log_level(LogLevel.INFO)


if len(sys.argv) > 1:
    bkgType = sys.argv[1]
    f = open('inputFiles/' + bkgType + '.txt', 'r')
    fileList = f.read()
    f.close()
    if not os.path.isfile(fileList[:-1]):
        sys.exit('Could not find root file : ' + fileList[:-1])
    print('Running over file ' + fileList[:-1])
elif len(sys.argv) == 1:
    fileList = \
        ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
         'mdst_000001_prod00000001_task00000001.root'

         ]
    bkgType = 'notSpecified'


if len(sys.argv) > 1:
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    inputMdstList('default', fileList)


# create and fill pion and kaon ParticleLists
# second argument are the selection criteria: '' means no cut, take all
fillParticleList('pi+:all', '')
fillParticleList('K+:all', '')


# B+ to D(->h+h-)h+ Skim
from BtoDh_hh_List import *

loadD0bar()
BtoDhList = BsigToDhTohhList()

skimOutputUdst('outputFiles/BtoDh_hh_' + bkgType, BtoDhList)
summaryOfLists(BtoDhList)

process(analysis_main)

# print out the summary
print(statistics)
