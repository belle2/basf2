#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FullEventInterpretation import *
from basf2 import *
from modularAnalysis import *

pid_variables = ['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
                 'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                 'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                 'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH']
kinematic_variables = ['p', 'pt', 'p_CMS', 'pt_CMS', 'M', 'E', 'px', 'py', 'pz', 'px_CMS', 'py_CMS', 'pz_CMS']
vertex_variables = ['dr', 'dz', 'dx', 'dy', 'chiProb', 'cosAngleBetweenMomentumAndVertexVector', 'significanceOfDistance', 'distance']

B_variables = ['dr', 'dz', 'dx', 'dy', 'distance', 'chiProb', 'significanceOfDistance', 'deltaE', 'cosAngleBetweenMomentumAndVertexVector',
               'daughter(0, dz)', 'daughter(0, distance)', 'daughter(1, dz)', 'daughter(1, distance)', 'decayAngle(0)', 'decayAngle(1)',
               'daughterAngle(0,1)', 'daughterInvariantMass(0,1)']

postCut = Particle.PostCutConfiguration(
    value=0.1
)

postCutSoft = Particle.PostCutConfiguration(
    value=0.01
)


particles = []

################### Charged FSP ###############################
mva_chargedFSP = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=pid_variables + kinematic_variables + ['chiProb'],
    target='isSignal'
)

particles.append(Particle('pi+', mva_chargedFSP, postCutConfig=postCut))
particles.append(Particle('e+', mva_chargedFSP, postCutConfig=postCut))
particles.append(Particle('mu+', mva_chargedFSP, postCutConfig=postCut))
particles.append(Particle('K+', mva_chargedFSP, postCutConfig=postCut))

################## GAMMA ############################

mva_gamma = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + ['clusterReg', 'goodGamma', 'goodGammaUnCal', 'clusterNHits', 'clusterTrackMatch', 'clusterE9E25'],
    target='isSignal'
)

particles.append(Particle('gamma', mva_gamma, postCutConfig=postCut))


################## PI0 ###############################
mva_pi0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + ['daughterAngle(0,1)'],
    target='isSignal'
)

pre_pi0 = Particle.PreCutConfiguration(
    variable='M',
    binning=(500, 0.08, 0.18),
    efficiency=0.95,
    purity=0.0001,
    userCut=''
)

particles.append(Particle('pi0', mva_pi0, pre_pi0, postCut).addChannel(['gamma', 'gamma']))

################### KS0 ###############################
mva_KS0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + vertex_variables + ['daughterAngle(0,1)'],
    target='isSignal'
)

mva_KS0_pi0pi0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'M', 'E', 'daughterAngle(0,1)'],
    target='isSignal'
)

pre_KS0 = Particle.PreCutConfiguration(
    variable='M',
    binning=(500, 0.3, 0.7),
    efficiency=0.95,
    purity=0.0001,
    userCut=''
)

p = Particle('K_S0', mva_KS0, pre_KS0, postCut)
p.addChannel(['pi+', 'pi-'])
p.addChannel(['pi0', 'pi0'], mva_KS0_pi0pi0)
particles.append(p)

######################## D0 #########################
mva_D0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal'
)

mva_D0_withoutVertex = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
    target='isSignal'
)


pre_D0 = Particle.PreCutConfiguration(
    variable='M',
    binning=(500, 1.5, 2.0),
    efficiency=0.95,
    purity=0.001,
    userCut=''
)

p = Particle('D0', mva_D0, pre_D0, postCutSoft)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi0'])
p.addChannel(['K-', 'pi+', 'pi0', 'pi0'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['pi-', 'pi+'])
p.addChannel(['pi-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['pi-', 'pi+', 'pi0'])
p.addChannel(['pi-', 'pi+', 'pi0', 'pi0'])
p.addChannel(['K_S0', 'pi0'], mva_D0_withoutVertex)
p.addChannel(['K_S0', 'pi+', 'pi-'])
p.addChannel(['K_S0', 'pi+', 'pi-', 'pi0'])
p.addChannel(['K-', 'K+'])
p.addChannel(['K-', 'K+', 'pi0'])
p.addChannel(['K-', 'K+', 'K_S0'])
particles.append(p)

######################## D+ #########################

mva_DPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal'
)

mva_DPlus_withoutVertex = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
    target='isSignal'
)


pre_DPlus = Particle.PreCutConfiguration(
    variable='M',
    binning=(500, 1.5, 2.0),
    efficiency=0.95,
    purity=0.001,
    userCut=''
)

p = Particle('D+', mva_DPlus, pre_DPlus, postCutSoft)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
p.addChannel(['pi+', 'pi0'], mva_DPlus_withoutVertex)
p.addChannel(['pi+', 'pi+', 'pi-'])
p.addChannel(['pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['K_S0', 'pi+'])
p.addChannel(['K_S0', 'pi+', 'pi0'])
p.addChannel(['K_S0', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K+', 'K_S0', 'K_S0'])

particles.append(p)

######################## D*+ #########################

mva_DStarPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal'
)

mva_DStarPlus_withoutVertex = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
    target='isSignal'
)

pre_DStarPlus = Particle.PreCutConfiguration(
    variable='Q',
    binning=(500, 0, 1),
    efficiency=0.95,
    purity=0.001,
    userCut=''
)

p = Particle('D*+', mva_DStarPlus, pre_DStarPlus, postCutSoft)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'], mva_DStarPlus_withoutVertex)
p.addChannel(['D+', 'gamma'], mva_DStarPlus_withoutVertex)
particles.append(p)

######################## D*0 #########################
mva_DStar0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
    target='isSignal'
)

pre_DStar0 = Particle.PreCutConfiguration(
    variable='Q',
    binning=(500, 0, 1),
    efficiency=0.95,
    purity=0.001,
    userCut=''
)

p = Particle('D*0', mva_DStar0, pre_DStar0, postCutSoft)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
particles.append(p)

######################## D_s+ #########################

mva_DS = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal'
)

pre_DS = Particle.PreCutConfiguration(
    variable='M',
    binning=(500, 1.6, 2.2),
    efficiency=0.95,
    purity=0.001,
    userCut=''
)


p = Particle('D_s+', mva_DS, pre_DS, postCutSoft)
p.addChannel(['K+', 'K_S0'])
p.addChannel(['K+', 'pi+', 'pi-'])
p.addChannel(['K+', 'K-', 'pi+'])
p.addChannel(['K+', 'K-', 'pi+', 'pi0'])
p.addChannel(['K+', 'K_S0', 'pi+', 'pi-'])
p.addChannel(['K-', 'K_S0', 'pi+', 'pi+'])
p.addChannel(['K+', 'K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['pi+', 'pi+', 'pi-'])
p.addChannel(['K_S0', 'pi+'])
p.addChannel(['K_S0', 'pi+', 'pi0'])
particles.append(p)


######################## DStar_s+ #########################

mva_DStarS = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables,
    target='isSignal'
)

pre_DStarS = Particle.PreCutConfiguration(
    variable='Q',
    binning=(500, 0, 1),
    efficiency=0.95,
    purity=0.001,
    userCut=''
)

p = Particle('D_s*+', mva_DStarS, pre_DStarS, postCutSoft)
p.addChannel(['D_s+', 'gamma'])
p.addChannel(['D_s+', 'pi0'])
particles.append(p)

######################## J/psi #########################

mva_J = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal'
)

pre_J = Particle.PreCutConfiguration(
    variable='M',
    binning=(500, 2.5, 3.5),
    efficiency=0.95,
    purity=0.001,
    userCut=''
)

p = Particle('J/psi', mva_J, pre_J)
p.addChannel(['e+', 'e-'])
p.addChannel(['mu+', 'mu-'])
particles.append(p)

################### B+ ##############################
mva_BPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
    target='isSignal'
)

mva_BPlus_low_stat = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=5:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
    target='isSignal'
)

pre_BPlus = Particle.PreCutConfiguration(
    variable='daughterProductOf(extraInfo(SignalProbability))',
    binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
    efficiency=0.95,
    purity=0.0001,
    userCut='Mbc > 5.2 and abs(deltaE) < 0.5 and nRemainingTracksInRestOfEvent == 0'
)

p = Particle('B+', mva_BPlus, pre_BPlus)
p.addChannel(['anti-D0', 'pi+'])
p.addChannel(['anti-D0', 'pi+', 'pi0'])
p.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-'])
p.addChannel(['anti-D0', 'D+'])
#p.addChannel(['anti-D0', 'D+', 'K_S0'])
#p.addChannel(['anti-D*0', 'D+', 'K_S0'])
#p.addChannel(['anti-D0', 'D*+', 'K_S0'])
#p.addChannel(['anti-D*0', 'D*+', 'K_S0'])
p.addChannel(['anti-D0', 'D0', 'K+'])
p.addChannel(['anti-D*0', 'D0', 'K+'])
p.addChannel(['anti-D0', 'D*0', 'K+'])
p.addChannel(['anti-D*0', 'D*0', 'K+'])
p.addChannel(['D_s+', 'anti-D0'])
p.addChannel(['anti-D*0', 'pi+'])
p.addChannel(['anti-D*0', 'pi+', 'pi0'])
p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-'])
p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['D_s*+', 'anti-D0'])
p.addChannel(['D_s+', 'anti-D*0'])
p.addChannel(['anti-D0', 'K+'])
p.addChannel(['D-', 'pi+', 'pi+'])
p.addChannel(['J/psi', 'K+'], mva_BPlus_low_stat)
p.addChannel(['J/psi', 'K+', 'pi+', 'pi-'])
p.addChannel(['J/psi', 'K+', 'pi0'])
p.addChannel(['J/psi', 'K_S0', 'pi+'])
particles.append(p)

mva_BPlusSemileptonic = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
    target='isSignalAcceptMissingNeutrino'
)

pre_BPlusSemileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(extraInfo(SignalProbability))',
    binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
    efficiency=0.95,
    purity=0.0001,
    userCut='nRemainingTracksInRestOfEvent == 0'
)

p = Particle('B+:semileptonic', mva_BPlusSemileptonic, pre_BPlusSemileptonic)
p.addChannel(['anti-D0', 'e+'])
p.addChannel(['anti-D0', 'mu+'])
p.addChannel(['anti-D*0', 'e+'])
p.addChannel(['anti-D*0', 'mu+'])
p.addChannel(['D-', 'pi+', 'e+'])
p.addChannel(['D-', 'pi+', 'mu+'])
p.addChannel(['D*-', 'pi+', 'e+'])
p.addChannel(['D*-', 'pi+', 'mu+'])
particles.append(p)

##################################  B0 #################################
mva_B0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
    target='isSignal'
)

pre_B0 = Particle.PreCutConfiguration(
    variable='daughterProductOf(extraInfo(SignalProbability))',
    binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
    efficiency=0.95,
    purity=0.0001,
    userCut='Mbc > 5.2 and abs(deltaE) < 0.5 and nRemainingTracksInRestOfEvent == 0'
)

p = Particle('B0', mva_B0, pre_B0)
p.addChannel(['D-', 'pi+'])
p.addChannel(['D-', 'pi0', 'pi+'])
p.addChannel(['D-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['anti-D0', 'pi+', 'pi-'])
p.addChannel(['D-', 'D0', 'K+'])
p.addChannel(['D-', 'D*0', 'K+'])
p.addChannel(['D*-', 'D0', 'K+'])
p.addChannel(['D*-', 'D*0', 'K+'])
#p.addChannel(['D-', 'D+', 'K_S0'])
#p.addChannel(['D*-', 'D+', 'K_S0'])
#p.addChannel(['D-', 'D*+', 'K_S0'])
#p.addChannel(['D*-', 'D*+', 'K_S0'])
p.addChannel(['D_s+', 'D-'])
p.addChannel(['D*-', 'pi+'])
p.addChannel(['D*-', 'pi0', 'pi+'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['D_s*+', 'D-'])
p.addChannel(['D_s+', 'D*-'])
p.addChannel(['D_s*+', 'D*-'])
p.addChannel(['J/psi', 'K_S0'])
p.addChannel(['J/psi', 'K+', 'pi-'])
p.addChannel(['J/psi', 'K_S0', 'pi+', 'pi-'])
particles.append(p)

mva_B0Semileptonic = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(extraInfo(SignalProbability))'] + B_variables,
    target='isSignalAcceptMissingNeutrino'
)

pre_B0Semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(extraInfo(SignalProbability))',
    binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
    efficiency=0.95,
    purity=0.0001,
    userCut='nRemainingTracksInRestOfEvent == 0'
)

p = Particle('B0:semileptonic', mva_B0Semileptonic, pre_B0Semileptonic)
p.addChannel(['D-', 'e+'])
p.addChannel(['D-', 'mu+'])
p.addChannel(['D*-', 'e+'])
p.addChannel(['D*-', 'mu+'])
p.addChannel(['anti-D0', 'pi-', 'e+'])
p.addChannel(['anti-D0', 'pi-', 'mu+'])
p.addChannel(['anti-D*0', 'pi-', 'e+'])
p.addChannel(['anti-D*0', 'pi-', 'mu+'])
particles.append(p)

selection_path = create_path()
selection_path.add_module(register_module('RootInput'))
selection_path.add_module(register_module('ParticleLoader'))
selectParticle('mu+', 'muid > 0.6 and nTracks <= 12', persistent=True, path=selection_path)
reconstructDecay('tau+ ->  mu+', '', persistent=True, path=selection_path)
matchMCTruth('tau+', path=selection_path)
reconstructDecay('B+:sig -> tau+', '', persistent=True, path=selection_path)
matchMCTruth('B+:sig', path=selection_path)
buildRestOfEvent('B+:sig', path=selection_path)

analysis_path = create_path()
analysis_path.add_module(register_module('RootOutput'))

main = FullEventInterpretation(selection_path, analysis_path, particles)

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
