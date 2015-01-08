#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FEI import *
from basf2 import *

pid_variables = ['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
                 'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                 'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                 'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH']
kinematic_variables = ['p', 'pt', 'p_CMS', 'pt_CMS', 'M', 'E', 'px', 'py', 'pz', 'px_CMS', 'py_CMS', 'pz_CMS']
vertex_variables = ['dr', 'dz', 'dx', 'dy', 'chiProb', 'cosAngleBetweenMomentumAndVertexVector', 'significanceOfDistance', 'distance']

particles = []

################### Charged FSP ###############################
mva_chargedFSP = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=pid_variables + kinematic_variables + ['chiProb'],
    target='isSignal', targetCluster=1
)

particles.append(Particle('e+', mva_chargedFSP))
particles.append(Particle('pi+', mva_chargedFSP))
particles.append(Particle('mu+', mva_chargedFSP))

################## GAMMA ############################

mva_gamma = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + ['clusterReg', 'goodGamma', 'goodGammaUnCal', 'clusterNHits', 'clusterTrackMatch', 'clusterE9E25'],
    target='isSignal', targetCluster=1
)

particles.append(Particle('gamma', mva_gamma))


################## PI0 ###############################
mva_pi0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + ['daughterAngle(0,1)'],
    target='isSignal', targetCluster=1
)

pre_pi0 = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)


particles.append(Particle('pi0', mva_pi0, pre_pi0).addChannel(['gamma', 'gamma']))

################### KS0 ###############################
mva_KS0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + vertex_variables + ['daughterAngle(0,1)'],
    target='isSignal', targetCluster=1
)

mva_KS0_pi0pi0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'M', 'E', 'daughterAngle(0,1)'],
    target='isSignal', targetCluster=1
)

pre_KS0 = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('K_S0', mva_KS0, pre_KS0)
p.addChannel(['pi+', 'pi-'])
p.addChannel(['pi0', 'pi0'], mva_KS0_pi0pi0)
particles.append(p)

######################## J/psi #########################

mva_J = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_J = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('J/psi', mva_J, pre_J)
p.addChannel(['e+', 'e-'])
p.addChannel(['mu+', 'mu-'])
particles.append(p)


################### B0 ##############################

mva_B0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))', 'deltaE'] + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_B0 = Particle.PreCutConfiguration(
    variable='Mbc',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('B0', mva_B0, pre_B0)
p.addChannel(['J/psi', 'K_S0'])
particles.append(p)

main = create_path()
main.add_module(register_module('RootInput'))

fullEventInterpretation(main, particles)

main.add_module(register_module('ProgressBar'))
main.add_module(register_module('RootOutput'))

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
