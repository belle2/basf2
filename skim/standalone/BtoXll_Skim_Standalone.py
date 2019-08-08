#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *
from stdPhotons import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
import argparse
gb2_setuprel = 'release-03-02-00'
skimCode = encodeSkimName('BtoXll')


# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

path = Path()
fileList = get_test_file("mixedBGx1", "MC12")
inputMdstList('default', fileList, path=path)
loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi0s('loose', path=path)
stdPhotons('loose', path=path)
stdK('95eff', path=path)
stdPi('95eff', path=path)
stdE('95eff', path=path)
stdMu('95eff', path=path)
stdK('loose', path=path)
stdPi('loose', path=path)
stdKshorts(path=path)
loadStdLightMesons(path=path)

cutAndCopyList('gamma:ewp', 'gamma:loose', 'E > 0.1', path=path)
reconstructDecay('eta:ewp -> gamma:ewp gamma:ewp', '0.505 < M < 0.580', path=path)
# EWP Skim
from skim.ewp import B2XllList
XllList = B2XllList(path=path)
skimOutputUdst(skimCode, XllList, path=path)
summaryOfLists(XllList, path=path)


setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
