from basf2 import Path, process, B2INFO, statistics, find_file
from stdCharged import stdK, stdPi, stdPr
import modularAnalysis as ma

mypath = Path()
ma.inputMdst('default', find_file('B02Dpnbar_D2kpipi.root', data_type='examples', silent=False), path=mypath)

stdK('higheff', path=mypath)
stdPi('higheff', path=mypath)
stdPr('higheff', path=mypath)
ma.reconstructDecay('D-:sig -> K+:higheff pi-:higheff pi-:higheff', '', path=mypath)
ma.fillParticleList('anti-n0:sig', 'clusterE > 0.5 and isFromECL > 0', path=mypath)
ma.reconstructDecay('@Xsd:Dp -> D-:sig p+:higheff', '', chargeConjugation=False, path=mypath)
ma.reconstructDecay('B0:Dpnbar -> Xsd:Dp anti-n0:sig', '', chargeConjugation=False, path=mypath)
ma.neutralHadron4MomentumCalculator('B0:Dpnbar', path=mypath)
ma.applyCuts('B0:Dpnbar', 'deltaE < 0.5', path=mypath)
ma.variablesToNtuple('B0:Dpnbar', ['deltaE', 'M'], filename='test_NeutralHadron4MomentumCalculator.root', path=mypath)

process(mypath)
B2INFO(statistics)
