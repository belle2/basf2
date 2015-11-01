#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Minimal FEI example which reconstructs B+ from D*0 and D0 using pi+, pi0, K+ and gamma
# Number of channels is highly reduced and chosen cuts are hard to speed up the training
# Train on charged generic MC to see if you're setup works!

from fei import *
from basf2 import *
from modularAnalysis import *

particles = []

postCut = PostCutConfiguration(
    value=0.1
)


# ################## Charged FSP ###############################
mva_chargedFSP = MVAConfiguration(
    variables=['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
               'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
               'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
               'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH',
               'p', 'pt', 'pz',
               'dr', 'dz', 'chiProb'],
    target='isPrimarySignal',
)

pre_chargedFSP = PreCutConfiguration(
    variable='dr',
    # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
    binning=(100, 0.0, 10.0),
    efficiency=0.90,
    purity=0.001,
)

user_chargedFSP = UserCutConfiguration('[dr < 2] and [dz < 4]')

particles.append(Particle('pi+', mva_chargedFSP, pre_chargedFSP, user_chargedFSP,
                          postCutConfig=postCut).addChannel(['pi+:FSP']))
particles.append(Particle('K+', mva_chargedFSP, pre_chargedFSP, user_chargedFSP,
                          postCutConfig=postCut).addChannel(['K+:FSP']))

# ################# GAMMA ############################

mva_gamma = MVAConfiguration(
    variables=['clusterReg', 'goodGamma', 'goodGammaUnCal',
               'clusterNHits', 'clusterTiming', 'clusterE9E25',
               'pt', 'E', 'pz'],
    target='isPrimarySignal',
)

pre_gamma = PreCutConfiguration(
    variable='goodGamma',
    # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
    binning=(100, 0.0, 1.0),
    efficiency=0.90,
    purity=0.001,
)

user_gamma = UserCutConfiguration('E > 0.1')

particles.append(Particle('gamma', mva_gamma, pre_gamma, user_gamma, postCutConfig=postCut).addChannel(['gamma:FSP']))

# ################# PI0 ###############################
mva_pi0 = MVAConfiguration(
    variables=['M', 'daughter({},extraInfo(SignalProbability))', 'daughterAngle(0,1)',
               'pt', 'pz', 'E'],
    target='isSignal',
)

pre_pi0 = PreCutConfiguration(
    variable='M',
    # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
    binning=(500, 0.08, 0.18),
    efficiency=0.90,
    purity=0.001,
)

particles.append(
    Particle('pi0', mva_pi0, pre_pi0, postCutConfig=postCut).addChannel(['gamma', 'gamma']))

# ####################### D0 #########################

mva_D = MVAConfiguration(
    variables=['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
               'chiProb', 'daughter({}, chiProb)',
               'useRestFrame(daughter({}, p))',
               'useRestFrame(daughter({}, distance))',
               'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
               'daughterInvariantMass({},{})', 'daughterInvariantMass({},{},{})', 'daughterInvariantMass({},{},{},{})',
               'daughterInvariantMass({},{},{},{},{})', 'Q', 'extraInfo(decayModeID)'],
    target='isSignal',
)

pre_D0 = PreCutConfiguration(
    variable='M',
    # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
    binning=(500, 1.70, 1.95),
    efficiency=0.95,
    purity=0.001,
)

p = Particle('D0', mva_D, pre_D0, postCutConfig=postCut)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi0'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['pi-', 'pi+'])
p.addChannel(['pi-', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+'])
p.addChannel(['K-', 'K+', 'pi0'])
particles.append(p)


# ####################### D*0 #########################

pre_DStar0 = PreCutConfiguration(
    variable='Q',
    # The range should include the signal peak, and the uniform part of the background (important for sPlot training!)
    binning=(500, 0, 0.3),
    efficiency=0.95,
    purity=0.001,
)

p = Particle('D*0', mva_D, pre_DStar0, postCutConfig=postCut)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
particles.append(p)


# ################## B+ ##############################
mva_BPlus = MVAConfiguration(
    variables=['daughterProductOf(extraInfo(SignalProbability))', 'daughter({},extraInfo(SignalProbability))',
               'chiProb', 'daughter({}, chiProb)',
               'useRestFrame(daughter({}, p))',
               'useRestFrame(daughter({}, distance))',
               'decayAngle({})', 'daughterAngle({},{})', 'cosAngleBetweenMomentumAndVertexVector',
               'dr', 'dz', 'dx', 'dy', 'distance', 'significanceOfDistance', 'deltaE', 'extraInfo(decayModeID)'],
    target='isSignal',
)

pre_BPlus = PreCutConfiguration(
    variable='daughterProductOf(extraInfo(SignalProbability))',
    binning=list(reversed([1.0 / (1.5 ** i) for i in range(0, 20)])),
    efficiency=0.90,
    purity=0.0001,
)

p = Particle('B+', mva_BPlus, pre_BPlus, UserCutConfiguration('Mbc > 5.2 and abs(deltaE) < 0.5'))
p.addChannel(['anti-D0', 'pi+'])
p.addChannel(['anti-D0', 'pi+', 'pi0'])
p.addChannel(['anti-D*0', 'pi+'])
p.addChannel(['anti-D*0', 'pi+', 'pi0'])
p.addChannel(['anti-D0', 'K+'])
particles.append(p)


path = create_path()
path.add_module('RootInput')
feistate = fullEventInterpretation(None, path, particles)

if feistate.is_trained:
    open('FEI_finished_its_training', 'a').close()

# show constructed path
print(feistate.path)

process(feistate.path)

B2WARNING('event() statistics:')
print(statistics)
