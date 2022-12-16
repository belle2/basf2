# @cond
import json
import basf2 as b2
import modularAnalysis as ma
import vertex as vx

runningOnMC = snakemake.params.runningOnMC

inputjson = snakemake.input.inputfileList
outname = snakemake.output[0]

mypath = b2.create_path()
with open(inputjson) as f:
    inputMdstList = json.load(f)

ma.inputMdstList(filelist=inputMdstList, path=mypath)

ma.fillParticleList(decayString='K+:my',  cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance and kaonID > 0.01", path=mypath)
ma.fillParticleList(decayString='pi+:my', cut="dr < 0.5 and abs(dz) < 3 and thetaInCDCAcceptance", path=mypath)

ma.reconstructDecay(decayString="D-:K2Pi -> K+:my pi-:my pi-:my", cut="1.844 < M < 1.894", path=mypath)

ma.reconstructDecay(decayString='B0:PiD-toK2Pi -> D-:K2Pi pi+:my', cut='5.0 < Mbc and abs(deltaE) < 1.0', path=mypath)
vx.treeFit('B0:PiD-toK2Pi', 0, path=mypath, updateAllDaughters=False, ipConstraint=True, massConstraint=[411])

if(runningOnMC):
    ma.matchMCTruth(list_name='B0:PiD-toK2Pi', path=mypath)

some_variables = ['Mbc', 'deltaE']
ma.variablesToNtuple(decayString='B0:PiD-toK2Pi', variables=some_variables,
                     filename=outname,  path=mypath, treename='BtoPiDtoKPiPi')

b2.process(mypath)
# @endcond
