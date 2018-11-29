#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#

# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdV0s import *
from stdPi0s import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-01'

import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyNeutrals')

c2bnpath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=c2bnpath)

"""
if len(sys.argv)>1:
  bkgType=sys.argv[1]
  f=open('inputFiles/'+bkgType+'.txt','r')
  fileList=f.read()
  f.close()
  if not os.path.isfile(fileList[:-1]):
    sys.exit('Could not find root file : ' +fileList[:-1])
  print('Running over file ' + fileList[:-1])
elif len(sys.argv)==1:
  fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'

    ]
  bkgType='old'


if len(sys.argv)>1:
  inputMdstList('MC9',fileList[:-1], path=c2bnpath)
elif len(sys.argv)==1:
  inputMdstList('MC9',fileList, path=c2bnpath)
"""

stdPi('loose', path=c2bnpath)
stdK('loose', path=c2bnpath)
stdE('loose', path=c2bnpath)
stdMu('loose', path=c2bnpath)
stdPi('all', path=c2bnpath)
stdK('all', path=c2bnpath)
stdE('all', path=c2bnpath)
stdMu('all', path=c2bnpath)
stdKshorts(path=c2bnpath)
loadStdSkimPi0(path=c2bnpath)

from skim.charm import DstToD0Neutrals
DstList = DstToD0Neutrals(c2bnpath)
skimOutputUdst(skimCode, DstList, path=c2bnpath)

summaryOfLists(DstList, path=c2bnpath)

setSkimLogging(path=c2bnpath)
process(path=c2bnpath)

print(statistics)
