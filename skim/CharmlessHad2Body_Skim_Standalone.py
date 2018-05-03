#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 2 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 03 May 2018
#######################################################

import ROOT
ROOT.gROOT.SetBatch(True)
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
from softwaretrigger import *

from stdLightMesons import *
from stdCharged import stdLoosePi
from stdCharged import stdLooseK
from stdPi0s import loadStdSkimPi0
from stdPi0s import stdPi0s
# from stdFSParticles import stdKs
from stdV0s import stdKshorts
from stdPhotons import stdPhotons

from CharmlessHad2Body_List import *

set_log_level(LogLevel.INFO)

gb2_setuprel = "release-02-00-00"

if len(sys.argv) > 1:
    bkgType = sys.argv[1]
    f = open('inputFiles/MC9/' + bkgType + '.txt', 'r')
    fileList = f.read()
    f.close()
    if not os.path.isfile(fileList[:-1]):
        sys.exit('Could not find root file : ' + fileList[:-1])
    print('Running over file ' + fileList[:-1])
    inputMdstList('default', fileList[:-1])
elif len(sys.argv) == 1:
    sys.exit('No background type specified.')

# Load particle lists
stdPhotons('loose')  # gamma:loose
stdLooseK()  # K+/-
stdKshorts()  # K_S0
stdLoosePi()  # pi+/-
stdPi0s('loose')
stdPi0s('all')
loadStdSkimPi0()  # pi0:skim
loadStdLightMesons()  # rho+/-, rho0, K*+/-, K*0, phi, f_0, omega, eta, eta'
add_hlt_software_trigger(analysis_main)
# Perform skim
Had2BodyList = CharmlessHad2BodyB0List() + CharmlessHad2BodyBmList()
skimOutputUdst('outputFiles/MC9/CharmlessHad2Body_' + bkgType, Had2BodyList)
summaryOfLists(Had2BodyList)

process(analysis_main)
print(statistics)
