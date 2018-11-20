#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Prepare all skims at once
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from skim.standardparticles.charm import *
from skim.standardparticles.lightmesons import *
from skim.standardparticles.dileptons import *
set_log_level(LogLevel.INFO)

from skimExpertFunctions import *


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)


loadStdCharged()
stdPi0s('loose')
stdPhotons('loose')
loadStdKS()
loadStdLightMesons()
loadStdSkimPi0()
loadStdSkimPhoton()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()
loadStdDiLeptons(True)

cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4')


def add_skim(label, lists):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.
    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists)
    summaryOfLists(lists)


from skim.dark import SinglePhotonDarkList
darklist = SinglePhotonDarkList()
add_skim('SinglePhotonDark', darklist)


from skim.dark import LFVZpInvisibleList
add_skim('LFVZpInvisible', LFVZpInvisibleList())

from skim.dark import LFVZpVisibleList
add_skim('LFVZpVisible', LFVZpVisibleList())
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
