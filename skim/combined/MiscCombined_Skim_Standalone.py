#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run TCPV skim
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK, stdPr, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file

skimpath = b2.Path()
fileList = get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=skimpath)


stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdPi0s('loose', path=skimpath)
stdPhotons('loose', path=skimpath)
stdKshorts(path=skimpath)
loadStdLightMesons(path=skimpath)
stdPhotons('all', path=skimpath)
loadStdSkimPi0(path=skimpath)
loadStdSkimPhoton(path=skimpath)
loadStdDiLeptons(True, path=skimpath)
loadStdLightMesons(path=skimpath)

ma.cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=skimpath)


# TCPV Skim
from skim.tcpv import TCPVList
add_skim('TCPV', TCPVList(path=skimpath), path=skimpath)

setSkimLogging(path=skimpath)
b2.process(path=skimpath)

# print out the summary
print(b2.statistics)
