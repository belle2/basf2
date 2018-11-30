#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run all dark skims at once
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *

from skimExpertFunctions import setSkimLogging, encodeSkimName, add_skim


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

darkskimpath = Path()
inputMdstList('MC9', fileList, path=darkskimpath)


stdPi('all', path=darkskimpath)
stdPi('loose', path=darkskimpath)
stdK('all', path=darkskimpath)
stdK('loose', path=darkskimpath)
stdPi0s('loose', path=darkskimpath)
stdPhotons('loose', path=darkskimpath)
stdKshorts(path=darkskimpath)
loadStdLightMesons(path=darkskimpath)
loadStdSkimPi0(path=darkskimpath)
loadStdSkimPhoton(path=darkskimpath)

loadStdD0(path=darkskimpath)
loadStdDplus(path=darkskimpath)
loadStdDstar0(path=darkskimpath)
loadStdDstarPlus(path=darkskimpath)
loadStdDiLeptons(True, path=darkskimpath)

cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=darkskimpath)


from skim.dark import ALP3GammaList
add_skim('ALP3Gamma', ALP3GammaList(path=darkskimpath))


from skim.dark import LFVZpInvisibleList
add_skim('LFVZpInvisible', LFVZpInvisibleList(path=darkskimpath))

from skim.dark import LFVZpVisibleList
add_skim('LFVZpVisible', LFVZpVisibleList(path=darkskimpath))


from skim.dark import SinglePhotonDarkList
add_skim('SinglePhotonDark', SinglePhotonDarkList(path=darkskimpath))
setSkimLogging(skim_path=darkskimpath)
process(darkskimpath)

# print out the summary
print(statistics)
