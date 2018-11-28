#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmlessSkims in one _standalone
#
######################################################

from basf2 import *
from modularAnalysis import *
from skim.standardlists.lightmesons import loadStdLightMesons
from stdCharged import stdPi
from stdCharged import stdK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from stdPhotons import stdPhotons
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging

gb2_setuprel = "release-02-00-00"

# Create skim path
btocharmlesspath = Path()

fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]
inputMdstList('MC9', fileList, path=btocharmlesspath)

# Load particle lists
stdPhotons('loose', path=btocharmlesspath)
stdK('loose', path=btocharmlesspath)
stdKshorts(path=btocharmlesspath)
stdPi('loose', path=btocharmlesspath)
stdPi0s('loose', path=btocharmlesspath)
stdPi0s('all', path=btocharmlesspath)
loadStdSkimPi0(path=btocharmlesspath)
loadStdLightMesons(path=btocharmlesspath)

# Import skim decay mode lists and perform skim
from skim.btocharmless import CharmlessHad2BodyB0List, CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyB0List(path=btocharmlesspath) + CharmlessHad2BodyBmList(path=btocharmlesspath)
add_skim("CharmlessHad2Body", Had2BodyList)

from skim.btocharmless import CharmlessHad3BodyB0List, CharmlessHad3BodyBmList
Had3BodyList = CharmlessHad3BodyB0List(path=btocharmlesspath) + CharmlessHad3BodyBmList(path=btocharmlesspath)
add_skim('CharmlessHad3Body', Had3BodyList)

setSkimLogging()
process(btocharmlesspath)

# print out the summary
print(statistics)
