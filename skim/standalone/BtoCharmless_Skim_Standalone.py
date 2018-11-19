#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmSkims in one _standalone
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *
from stdLightMesons import *
from stdDiLeptons import *
set_log_level(LogLevel.INFO)
import os
import sys
import glob
from skimExpertFunctions import *


fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]
inputMdstList('MC9', fileList)


loadStdCharged()


# Load particle lists
stdPhotons('loose')
stdLooseK()
stdKshorts()
stdLoosePi()
stdPi0s('loose')
stdPi0s('all')
loadStdSkimPi0()
loadStdLightMesons()


def add_skim(label, lists):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.

    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists)
    summaryOfLists(lists)


from skim.btocharmless import CharmlessHad2BodyB0List, CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyB0List() + CharmlessHad2BodyBmList()
add_skim("CharmlessHad2Body", Had2BodyList)

# Import skim decay mode lists and perform skim
from skim.btocharmless import CharmlessHad3BodyB0List, CharmlessHad3BodyBmList
Had3BodyList = CharmlessHad3BodyB0List() + CharmlessHad3BodyBmList()
add_skim('CharmlessHad3Body', Had3BodyList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
