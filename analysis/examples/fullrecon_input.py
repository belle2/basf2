#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FullEventInterpretation import *
from basf2 import *

chargedTrackVars = [
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
    'eid_ECL',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'Kid_ARICH',
    'prid',
    'prid_dEdx',
    'prid_TOP',
    'prid_ARICH',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'muid_ARICH',
    'p',
    'pt',
    'p_CMS',
    'pt_CMS',
    'chiProb']

#method = (
#    'BDTGradient_100', 'BDT',
#    '!H:CreateMVAPdfs:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:'
#    'UseBaggedGrad:GradBaggingFraction=0.5:nCuts=10:MaxDepth=2'
#)

#some methods for C&P:
#name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
#name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
mvaConfig_chargedFSP = Particle.MVAConfiguration(
    name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=chargedTrackVars,
    target='isSignal', targetCluster=1
)

mvaConfig_gamma = Particle.MVAConfiguration(
    name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['p', 'pt', 'clusterE9E25'],
    target='isSignal', targetCluster=1
)

mvaConfig_pi0 = Particle.MVAConfiguration(
    name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['p', 'pt', 'daughterAngle', 'M'],
    target='isSignal', targetCluster=1
)

mvaConfig_D = Particle.MVAConfiguration(
    #name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt', 'p_CMS', 'pt_CMS', 'M'],
    target='isSignal', targetCluster=1
)

mvaConfig_B = Particle.MVAConfiguration(
    #name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt', 'p_CMS', 'pt_CMS'],
    target='isSignal', targetCluster=1
)

preCutConfiguration_nonFSP = Particle.PreCutConfiguration(
    #variable='daughterProductOfSignalProbability',
    variable='Mass',
    efficiency=0.7
)


particles = []
particles.append(Particle('pi+', mvaConfig_chargedFSP, explicitCuts=['piid 0.1:']))
particles.append(Particle('K+', mvaConfig_chargedFSP, explicitCuts=['Kid 0.1:']))
particles.append(Particle('gamma', mvaConfig_gamma, explicitCuts=['goodGamma 0.1:']))  # TODO: not exactly soft

p = Particle('pi0', mvaConfig_pi0, preCutConfiguration_nonFSP)
p.addChannel(['gamma', 'gamma'])
particles.append(p)

p = Particle('D0', mvaConfig_D, preCutConfiguration_nonFSP)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K-', 'pi+', 'pi0'])
particles.append(p)

p = Particle('D+', mvaConfig_D, preCutConfiguration_nonFSP)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
particles.append(p)

p = Particle('D*+', mvaConfig_D, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
particles.append(p)

p = Particle('D*0', mvaConfig_D, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
particles.append(p)

p = Particle('B+', mvaConfig_B, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
p.addChannel(['D*-', 'pi+', 'pi+'])
particles.append(p)

p = Particle('B0', mvaConfig_B, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D-', 'pi+'])
p.addChannel(['D-', 'pi0', 'pi+'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
particles.append(p)

main = create_path()
main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))

FullEventInterpretation(main, particles)

main.add_module(register_module('ProgressBar'))

# ntupler = register_module('VariablesToNtuple')
# ntupler.param('particleList', 'D+:e280f278828c609765c60979efadb682ec239d64')
# ntupler.param('variables', ['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt', 'p_CMS', 'pt_CMS', 'M'])
# main.add_module(ntupler)

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
