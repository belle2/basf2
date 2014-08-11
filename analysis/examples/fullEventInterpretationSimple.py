#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FullEventInterpretation import *
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

particles.append(Particle('pi+', mva_chargedFSP))
particles.append(Particle('K+', mva_chargedFSP))


######################## D0 #########################

mva_D0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_D0 = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D0', mva_D0, pre_D0)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
particles.append(p)

######################## D+ #########################

mva_DPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_DPlus = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D+', mva_DPlus, pre_DPlus)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+'])
particles.append(p)

######################## D*+ #########################

mva_DStarPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_DStarPlus = Particle.PreCutConfiguration(
    variable='Q',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_DStarPlus_semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D*+', mva_DStarPlus, pre_DStarPlus)
p.addChannel(['D0', 'pi+'])
particles.append(p)

################### B0 ##############################

mva_B0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'deltaE'] + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_B0 = Particle.PreCutConfiguration(
    variable='Mbc',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('B0', mva_B0, pre_B0)
p.addChannel(['D-', 'pi+'])
p.addChannel(['D*-', 'pi+'])
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
