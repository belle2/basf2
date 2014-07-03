#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FullEventInterpretation import *
from basf2 import *
import pdg

getFSPClusterMVAConfiguration = lambda name: Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb'] + map(lambda x: ''.join(x), zip(['eid', 'Kid', 'muid', 'prid'], ['', '_dEdx', '_TOP', '_ARICH'])),
    target='abs_mcPDG', targetCluster=abs(pdg.from_name(name))
)

mvaConfig_chargedFSP = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'p_CMS', 'pt_CMS', 'chiProb'] + map(lambda x: ''.join(x), zip(['eid', 'Kid', 'muid', 'prid'], ['', '_dEdx', '_TOP', '_ARICH'])),
    target='isSignalAcceptMissingNeutrino', targetCluster=1
)

mvaConfig_nonFSP = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    #name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt', 'p_CMS', 'pt_CMS'],
    target='isSignalAcceptMissingNeutrino', targetCluster=1
)

mvaConfig_gamma = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'clusterE9E25'],
    target='isSignalAcceptMissingNeutrino', targetCluster=1
)

mvaConfig_pi0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'daughterAngle', 'M'],
    target='isSignalAcceptMissingNeutrino', targetCluster=1
)

mvaConfig_B = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    #name='NeuroBayes', type='Plugin', config='!H:V:CreateMVAPdfs:NtrainingIter=0:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt'],  # no variables correlated with mass here!
    target='isSignalAcceptMissingNeutrino', targetCluster=1
)

preCutConfiguration_nonFSP = Particle.PreCutConfiguration(
    #variable='daughterProductOfSignalProbability',
    variable='Mass',
    efficiency=0.7
)

particles = []

particles.append(Particle('gamma', mvaConfig_gamma))
particles.append(Particle('mu+', mvaConfig_chargedFSP))
particles.append(Particle('e+', mvaConfig_chargedFSP))
particles.append(Particle('pi+', mvaConfig_chargedFSP))
particles.append(Particle('K+', mvaConfig_chargedFSP))

"""
particles.append(Particle('mu+', getFSPClusterMVAConfiguration('mu+')))
particles.append(Particle('e+', getFSPClusterMVAConfiguration('e+')))
particles.append(Particle('pi+', getFSPClusterMVAConfiguration('pi+')))
particles.append(Particle('K+', getFSPClusterMVAConfiguration('K+')))
"""

p = Particle('pi0', mvaConfig_pi0, preCutConfiguration_nonFSP)
p.addChannel(['gamma', 'gamma'])
p.addChannel(['gamma'], isIncomplete=True)
particles.append(p)

p = Particle('D0', mvaConfig_nonFSP, preCutConfiguration_nonFSP)
p.addChannel(['K-', 'pi+', 'pi0'])
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K-', 'e+'], isIncomplete=True)
p.addChannel(['K-', 'mu+'], isIncomplete=True)
particles.append(p)

p = Particle('D+', mvaConfig_nonFSP, preCutConfiguration_nonFSP)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
p.addChannel(['K-', 'pi+', 'e+'], isIncomplete=True)
p.addChannel(['K-', 'pi+', 'mu+'], isIncomplete=True)
particles.append(p)

p = Particle('D*+', mvaConfig_nonFSP, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
particles.append(p)

p = Particle('D*0', mvaConfig_nonFSP, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
particles.append(p)

p = Particle('B+', mvaConfig_B, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
p.addChannel(['D*-', 'pi+', 'pi+'])
p.addChannel(['D0', 'e+'], isIncomplete=True)
p.addChannel(['D0', 'mu+'], isIncomplete=True)
particles.append(p)

p = Particle('B0', mvaConfig_B, preCutConfiguration_nonFSP)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D-', 'pi+'])
p.addChannel(['D-', 'pi0', 'pi+'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['D-', 'e+'], isIncomplete=True)
p.addChannel(['D-', 'mu+'], isIncomplete=True)
particles.append(p)

main = create_path()
main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))
FullEventInterpretation(main, particles)
main.add_module(register_module('ProgressBar'))

print main

for m in main.modules():
    print_params(m)
process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
