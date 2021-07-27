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
ma.inputMdst('default', find_file('B02Dpnbar_D2kpipi.root', data_type='examples', silent=False), path=mypath)

stdK('higheff', path=mypath)
stdPi('higheff', path=mypath)
stdPr('higheff', path=mypath)
ma.reconstructDecay('D-:sig -> K+:higheff pi-:higheff pi-:higheff', 'abs(dM) < 0.05', path=mypath)
ma.fillParticleList('anti-n0:sig', 'clusterE > 0.5 and isFromECL > 0', path=mypath)
ma.reconstructDecayWithNeutralHadron('B0:Dpnbar -> D-:sig p+:higheff ^anti-n0:sig', '', chargeConjugation=False, path=mypath)
ma.applyCuts('B0:Dpnbar', 'deltaE < 0.5', path=mypath)
ma.variablesToNtuple('B0:Dpnbar', ['deltaE', 'M'], filename='test_NeutralHadron4MomentumCalculator.root', path=mypath)

process(mypath)
B2INFO(statistics)
