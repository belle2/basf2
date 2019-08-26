#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run all dark skims at once
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdMu, stdK, stdE
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *

from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file
import argparse
gb2_setuprel = 'release-03-02-00'

fileList = get_test_file("mixedBGx1", "MC12")

# Read optional --data argument
parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

darkskimpath = Path()
inputMdstList('default', fileList, path=darkskimpath)

stdMu('all', path=darkskimpath)
stdMu('loose', path=darkskimpath)
stdE('all', path=darkskimpath)
stdE('loose', path=darkskimpath)
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
add_skim('ALP3Gamma', ALP3GammaList(path=darkskimpath), path=darkskimpath)

from skim.dark import LFVZpInvisibleList
add_skim('LFVZpInvisible', LFVZpInvisibleList(path=darkskimpath), path=darkskimpath)

from skim.dark import LFVZpVisibleList
add_skim('LFVZpVisible', LFVZpVisibleList(path=darkskimpath), path=darkskimpath)

from skim.dark import SinglePhotonDarkList
add_skim('SinglePhotonDark', SinglePhotonDarkList(path=darkskimpath), path=darkskimpath)

from skim.dark import DimuonForDarkSearchesList as DimuonList
add_skim('DimuonForDarkSearches', DimuonList(path=darkskimpath), path=darkskimpath)

from skm.dark import ElectronMuonForDarkSearchesList as ElectronMuonList
add_skim('ElectronMuonForDarkSearches', ElectronMuonList(path=darkskimpath), path=darkskimpath)

from skm.dark import DielectronForDarkSearchesList as DielectronList
add_skim('DielectronForDarkSearches', DielectronList(path=darkskimpath), path=darkskimpath)

setSkimLogging(path=darkskimpath)
process(darkskimpath)

# print out the summary
print(statistics)
