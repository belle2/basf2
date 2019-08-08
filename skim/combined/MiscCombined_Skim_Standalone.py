#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run TCPV, Tau Generic and Tau LFV skims at once
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdPr, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file
import argparse
gb2_setuprel = 'release-03-02-00'

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

skimpath = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=skimpath)


stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdPr('loose', path=skimpath)
stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdPi0s('loose', path=skimpath)
stdPhotons('loose', path=skimpath)
stdKshorts(path=skimpath)
loadStdLightMesons(path=skimpath)
loadStdSkimPi0(path=skimpath)
loadStdSkimPhoton(path=skimpath)

loadStdD0(path=skimpath)
loadStdDplus(path=skimpath)
loadStdDstar0(path=skimpath)
loadStdDstarPlus(path=skimpath)
loadStdDiLeptons(True, path=skimpath)

cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=skimpath)


# BtoPi0Pi0 Skim
# from skim.btocharmless import BtoPi0Pi0List
# add_skim('BtoPi0Pi0', BtoPi0Pi0List(path=skimpath))
# Tau Skim
from skim.taupair import TauLFVList
add_skim('TauLFV', TauLFVList(path=skimpath), path=skimpath)

# TCPV Skim
from skim.tcpv import TCPVList
add_skim('TCPV', TCPVList(path=skimpath), path=skimpath)


# Tau Generic
from skim.taupair import TauList
add_skim('TauGeneric', TauList(path=skimpath), path=skimpath)


setSkimLogging(path=skimpath)
process(path=skimpath)

# print out the summary
print(statistics)
