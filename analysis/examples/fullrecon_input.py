#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

#from FullReconstruction import *
from FullReconstruction import *
from basf2 import *

chargedTrackVars = [
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
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
    'M',
    'chiProb']

DVars = ['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt', 'p_CMS', 'pt_CMS', 'M']
BVars = ['daughterProductOf(getExtraInfo(SignalProbability))', 'p', 'pt', 'p_CMS', 'pt_CMS', 'M']
method = ('FastBDT', 'Plugin', '!H:CreateMVAPdfs:!V:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')
#method = (
#    'BDTGradient_100', 'BDT',
#    '!H:CreateMVAPdfs:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:'
#    'UseBaggedGrad:GradBaggingFraction=0.5:nCuts=10:MaxDepth=2'
#)
#nb = (
#    'NeuroBayes', 'Plugin',
#    '!H:CreateMVAPdfs:V:NTrainingIter=50:TrainingMethod=BFGS'
#)

particles = []
particles.append(Particle('pi+', chargedTrackVars, method))
particles.append(Particle('K+', chargedTrackVars, method))
particles.append(Particle('gamma', ['p', 'pt', 'clusterE9E25'], method))
particles.append(Particle('pi0', ['p', 'pt', 'daughterAngle', 'M'], method))


p = Particle('D0', DVars, method).addChannel(['K-', 'pi+'])
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
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
particles.append(p)

main = create_path()
main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))

FullReconstruction(main, particles)

main.add_module(register_module('ProgressBar'))

# ntupler = register_module('VariableNtuple')
# ntupler.param('particleList', 'B+')
# ntupler.param('variables', ['p', 'pt', 'M', 'dM', 'Q', 'dQ', 'Mbc', 'deltaE',
#'nDaughters', 'flavor', 'tmptruth'])
# main.add_module(ntupler)

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
