#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# Charm skim lists with retentions. for all kinds of MC, lower
# than 3% are added to Charm_Skim_Standalone_Low.py, other
# charm skim lists are added to Charm_Skim_Standalone_High.py
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdMu, stdE, stdMu
from stdV0s import *
from stdPi0s import *
from skimExpertFunctions import *


gb2_setuprel = 'release-04-00-00'
set_log_level(LogLevel.INFO)

charmpath = Path()
fileList = [

    '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007392/s00/e1003/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000141_prod00007392_task10020000141.root'

]

inputMdstList('default', fileList, path=charmpath)


loadStdSkimPhoton(path=charmpath)
loadStdSkimPi0(path=charmpath)
stdPi('loose', path=charmpath)
stdK('loose', path=charmpath)
stdE('loose', path=charmpath)
stdMu('loose', path=charmpath)
stdPi('all', path=charmpath)
stdK('all', path=charmpath)
stdE('all', path=charmpath)
stdMu('all', path=charmpath)
stdPi('95eff', path=charmpath)
stdK('95eff', path=charmpath)
stdE('95eff', path=charmpath)
stdMu('95eff', path=charmpath)
stdKshorts(path=charmpath)
mergedKshorts(path=charmpath)


from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(charmpath)
add_skim("DstToD0Pi_D0ToHpJm", DstToD0PiD0ToHpJmList, path=charmpath)

from skim.charm import DstToD0PiD0ToHpHmKs
DstToD0PiD0ToHpHmKsList = DstToD0PiD0ToHpHmKs(charmpath)
add_skim("DstToD0Pi_D0ToHpHmKs", DstToD0PiD0ToHpHmKsList, path=charmpath)

from skim.charm import DstToD0PiD0ToHpJmEta
DstToD0PiD0ToHpJmEtaList = DstToD0PiD0ToHpJmEta(charmpath)
add_skim("DstToD0Pi_D0ToHpJmEta", DstToD0PiD0ToHpJmEtaList, path=charmpath)

from skim.charm import DstToD0PiD0ToKsOmega
DstList = DstToD0PiD0ToKsOmega(charmpath)
add_skim("DstToD0Pi_D0ToKsOmega", DstList, path=charmpath)

from skim.charm import DstToD0Neutrals
add_skim("DstToD0Pi_D0ToNeutrals", DstToD0Neutrals(charmpath), path=charmpath)

from skim.charm import CharmRare
DstToD0Pi_D0ToRareList = CharmRare(charmpath)
add_skim("DstToD0Pi_D0ToRare", DstToD0Pi_D0ToRareList, path=charmpath)

from skim.charm import CharmSemileptonic
CSLList = CharmSemileptonic(charmpath)
add_skim("DstToD0Pi_D0ToSemileptonic", CSLList, path=charmpath)

setSkimLogging(path=charmpath)
process(charmpath)

print(statistics)
