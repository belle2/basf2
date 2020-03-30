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

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPi0s import loadStdSkimPi0
from stdPhotons import loadStdSkimPhoton
from stdV0s import stdKshorts
import skimExpertFunctions as expert


b2.set_log_level(b2.LogLevel.INFO)

charmpath = b2.Path()
fileList = [

    '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007392/s00/e1003/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000141_prod00007392_task10020000141.root'

]

ma.inputMdstList('default', fileList, path=charmpath)


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
stdKshorts(path=charmpath)


from skim.charm import DstToD0PiD0ToHpJmPi0
DstToD0PiD0ToHpJmPi0List = DstToD0PiD0ToHpJmPi0(charmpath)
expert.add_skim("DstToD0Pi_D0ToHpJmPi0", DstToD0PiD0ToHpJmPi0List, path=charmpath)

from skim.charm import D0ToNeutrals

expert.add_skim('XToD0_D0ToNeutrals', D0ToNeutrals(path=charmpath), path=charmpath)


from skim.charm import DstToD0PiD0ToHpHmPi0
DstToD0PiD0ToHpHmPi0List = DstToD0PiD0ToHpHmPi0(path=charmpath)
expert.add_skim("DstToD0Pi_D0ToHpHmPi0", DstToD0PiD0ToHpHmPi0List, path=charmpath)

from skim.charm import DpToKsHp
DpToKsHpList = DpToKsHp(charmpath)
expert.add_skim("XToDp_DpToKsHp", DpToKsHpList, path=charmpath)

expert.setSkimLogging(path=charmpath)
b2.process(charmpath)

print(b2.statistics)
