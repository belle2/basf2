# @cond
import basf2 as b2
import modularAnalysis as ma
import vertex as vx
from extravariables import runningOnMC, outputfile
import udst

gbasf2_dataset = []  # gbasf2 will figure this out

mypath = b2.create_path()
ma.inputMdstList(filelist=gbasf2_dataset, path=mypath, entrySequences=['0:10'])

ma.fillParticleList(decayString='K+:my',  cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance and kaonID > 0.01", path=mypath)
ma.fillParticleList(decayString='pi+:my', cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance", path=mypath)

ma.reconstructDecay(decayString="D-:K2Pi -> K+:my pi-:my pi-:my", cut="1.5 < M < 2.2", path=mypath)

ma.reconstructDecay(decayString='B0:PiD-toK2Pi -> D-:K2Pi pi+:my', cut='5.0 < Mbc and abs(deltaE) < 1.0', path=mypath)
vx.treeFit('B0:PiD-toK2Pi', 0, path=mypath, updateAllDaughters=False, ipConstraint=True, massConstraint=[411])
ma.applyCuts('B0:PiD-toK2Pi', '5.2 < Mbc and abs(deltaE) < 0.5', path=mypath)

# dump in UDST format
udst.add_skimmed_udst_output(
    mypath,
    skimDecayMode='BtoPiD',
    skimParticleLists=['B0:PiD-toK2Pi'],
    mc=runningOnMC,
    outputFile=outputfile)
b2.process(mypath)
print(b2.statistics)
# @endcond
