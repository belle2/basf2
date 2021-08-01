##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import Path, process, B2INFO, statistics, find_file
from stdCharged import stdK, stdPi, stdPr
import modularAnalysis as ma

mypath = Path()
ma.inputMdst(
    environmentType='default',
    filename=find_file(
        'B02Dpnbar_D2kpipi.root',
        data_type='examples',
        silent=False),
    path=mypath)

stdK('higheff', path=mypath)
stdPi('higheff', path=mypath)
stdPr('higheff', path=mypath)
ma.reconstructDecay('D-:sig -> K+:higheff pi-:higheff pi-:higheff', 'abs(dM) < 0.05', path=mypath)
ma.fillParticleList('anti-n0:good', 'isFromECL > 0', path=mypath)
ma.reconstructDecayWithNeutralHadron('B0:sig -> D-:sig p+:higheff ^anti-n0:good', '', chargeConjugation=False, path=mypath)

fname = 'neutralHadron4MomentumCalculation.root'
ma.variablesToNtuple('B0:sig', ['deltaE', 'M', 'daughter(2,p)', 'daughter(2,clusterE)'], treename='B0', filename=fname, path=mypath)
ma.variablesToNtuple('anti-n0:good', ['p'], treename='nbar', filename=fname, path=mypath)

process(mypath)
B2INFO(statistics)
