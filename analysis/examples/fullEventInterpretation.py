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

#some methods for C&P:
#name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
#name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',

mvaConfig_chargedFSP = Particle.MVAConfiguration(
    #name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=chargedTrackVars,
    target='isSignal', targetCluster=1
)

mvaConfig_gamma = Particle.MVAConfiguration(
    #name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'clusterE9E25'],
    target='isSignal', targetCluster=1
)

mvaConfig_pi0 = Particle.MVAConfiguration(
    # name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'daughterAngle', 'M'],
    target='isSignal', targetCluster=1
)

mvaConfig_D = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    # name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt', 'p_CMS', 'pt_CMS', 'M'],
    target='isSignal', targetCluster=1
)

mvaConfig_B = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    # name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'deltaE'],
    target='isSignal', targetCluster=1
)

preCutConfiguration_nonFSP = Particle.PreCutConfiguration(
    #variable='daughterProductOf(getExtraInfo(SignalProbability))',
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

preCutConfiguration_DStar = Particle.PreCutConfiguration(
    #variable='daughterProductOf(getExtraInfo(SignalProbability))',
    variable='Q',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

partial = True

particles = []
particles.append(Particle('pi+', mvaConfig_chargedFSP))
particles.append(Particle('e+', mvaConfig_chargedFSP))
particles.append(Particle('mu+', mvaConfig_chargedFSP))
particles.append(Particle('K+', mvaConfig_chargedFSP))
particles.append(Particle('gamma', mvaConfig_gamma))

p = Particle('pi0', mvaConfig_pi0, preCutConfiguration_nonFSP)
p.addChannel(['gamma', 'gamma'])
if partial:
    p.addChannel(['gamma'], isIncomplete=True)
particles.append(p)

p = Particle('D0', mvaConfig_D, preCutConfiguration_nonFSP)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi0'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['pi-', 'pi+'])
p.addChannel(['pi-', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+'])
if partial:
    p.addChannel(['K-', 'e+'], isIncomplete=True)
    p.addChannel(['K-', 'mu+'], isIncomplete=True)
particles.append(p)

p = Particle('D+', mvaConfig_D, preCutConfiguration_nonFSP)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
if partial:
    p.addChannel(['K-', 'pi+', 'e+'], isIncomplete=True)
    p.addChannel(['K-', 'pi+', 'mu+'], isIncomplete=True)
particles.append(p)

p = Particle('D*+', mvaConfig_D, preCutConfiguration_DStar)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
particles.append(p)

p = Particle('D*0', mvaConfig_D, preCutConfiguration_DStar)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
particles.append(p)

p = Particle('B+', mvaConfig_B, preCutConfiguration_nonFSP)
p.addChannel(['anti-D0', 'pi+'])
p.addChannel(['anti-D0', 'pi+', 'pi0'])
p.addChannel(['anti-D0', 'K+'])
p.addChannel(['anti-D*0', 'pi+'])
p.addChannel(['anti-D*0', 'pi+', 'pi0'])
p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-'])
p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['D-', 'pi+', 'pi+'])
if partial:
    p.addChannel(['D0', 'e+'], isIncomplete=True)
    p.addChannel(['D0', 'mu+'], isIncomplete=True)
particles.append(p)

p = Particle('B0', mvaConfig_B, preCutConfiguration_nonFSP)
p.addChannel(['anti-D0', 'pi0'])
p.addChannel(['D-', 'pi+'])
p.addChannel(['D-', 'pi0', 'pi+'])
p.addChannel(['D-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-', 'pi+'])
p.addChannel(['D*-', 'pi0', 'pi+'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
if partial:
    p.addChannel(['D-', 'e+'], isIncomplete=True)
    p.addChannel(['D-', 'mu+'], isIncomplete=True)
particles.append(p)

main = create_path()
main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))

FullEventInterpretation(main, particles)

main.add_module(register_module('ProgressBar'))

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
