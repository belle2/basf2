#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from FullReconstruction import *
from basf2 import *

chargedTrackVars = [
    'eid_dEdx',
    'NB_eid_TOP',
    'NB_eid_ARICH',
    'Kid_dEdx',
    'NB_Kid_TOP',
    'NB_Kid_ARICH',
    'prid_dEdx',
    'NB_prid_TOP',
    'NB_prid_ARICH',
    'muid_dEdx',
    'NB_muid_TOP',
    'NB_muid_ARICH',
    'p',
    'pt',
    'chiProb',
]

DVars = ['prodChildProb', 'p', 'pt', 'p_CMS', 'pt_CMS']
BVars = ['prodChildProb', 'p', 'pt', 'p_CMS', 'pt_CMS']

method = (
    'BDTGradient_100', 'BDT',
    '!H:CreateMVAPdfs:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:'
    'UseBaggedGrad:GradBaggingFraction=0.5:nCuts=10:MaxDepth=2'
)
#nb = (
#    'NeuroBayes', 'Plugin',
#    '!H:CreateMVAPdfs:V:NTrainingIter=50:TrainingMethod=BFGS'
#)
#method = (
#    'FastBDT', 'Plugin',
#    '!H:CreateMVAPdfs:V'
#)


# Add FSP
particles = []
particles.append(Particle('gamma', ['p', 'pt', 'clusterE9E25'], method))
particles.append(Particle('pi+', chargedTrackVars, method))
particles.append(Particle('K+', chargedTrackVars, method))

p = Particle('pi0', ['E', 'p', 'pt', 'M', 'prodChildProb', 'daughterAngle'], method)
p.addChannel(['gamma', 'gamma'])
particles.append(p)

p = Particle('D0', DVars, method)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K-', 'pi+', 'pi0'])
particles.append(p)

p = Particle('D+', DVars, method)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
particles.append(p)

p = Particle('D*+', DVars, method)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
particles.append(p)

p = Particle('D*0', DVars, method)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
particles.append(p)

p = Particle('B+', BVars, method)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
p.addChannel(['D*-', 'pi+', 'pi+'])
particles.append(p)

p = Particle('B0', BVars, method)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D-', 'pi+'])
p.addChannel(['D-', 'pi0', 'pi+'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
#p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0']) # takes up most of the CPU time...
particles.append(p)

main = create_path()
main.add_module(register_module('RootInput'))

main.add_module(register_module('ParticleLoader'))

FullReconstruction(main, particles)

#ntupler = register_module('VariablesToNtuple')
#ntupler.param('particleList', 'K-')
#ntupler.param('variables', chargedTrackVars + ['SignalProbability'])
#main.add_module(ntupler)

main.add_module(register_module('ProgressBar'))

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
