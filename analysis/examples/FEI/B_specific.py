#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

from basf2 import *
from modularAnalysis import *

path = create_path()
path.add_module('RootInput')
path.add_module('Gearbox')
path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])

fillParticleList('mu+:sig', 'muid > 0.5 and dr < 1 and abs(dz) < 2', writeOut=True, path=path)
reconstructDecay('tau+:sig1 -> mu+:sig', '', 1, writeOut=True, path=path)

fillParticleList('e+:sig', 'eid > 0.5 and dr < 1 and abs(dz) < 2', writeOut=True, path=path)
reconstructDecay('tau+:sig2 -> e+:sig', '', 2, writeOut=True, path=path)

fillParticleList('pi+:sig', 'piid > 0.5 and dr < 1 and abs(dz) < 2 and 0.5 < useCMSFrame(p) < 2.8', writeOut=True, path=path)
reconstructDecay('tau+:sig3 -> pi+:sig', '', 3, writeOut=True, path=path)

copyLists('tau+:sig', ['tau+:sig1', 'tau+:sig2', 'tau+:sig3'], path=path)
matchMCTruth('tau+:sig', path=path)

reconstructDecay('B+:sig -> tau+:sig', '', writeOut=True, path=path)
matchMCTruth('B+:sig', path=path)
buildRestOfEvent('B+:sig', path=path)

applyCuts('B+:sig', '3 <= nRemainingTracksInEvent <= 12', path=path)
cleanROEMask = ('CleanROE', 'abs(d0) < 0.05 and abs(z0) < 0.1', 'clusterE9E25 > 0.905 and clusterTiming < 50')
appendROEMasks('B+:sig', [cleanROEMask], path=path)
applyCuts('B+:sig', 'ROE_deltae(CleanROE) > -0.38 and ROE_mbc(CleanROE) > 4.82', path=path)

path.add_module('MCDecayFinder', decayString='B+ ==> tau+ nu_tau', listName='B+:FEIMC', writeOut=True)

isSignal = 'isSignalAcceptMissingNeutrino'
signalMC = 'eventCached(countInList(B+:FEIMC))'
cut = '[[{mc} > 0 and {sig} == 1] or [{mc} == 0 and {sig} != 1]]'.format(mc=signalMC, sig=isSignal)
applyCuts('B+:sig', cut, path=path)

import fei
particles = get_default_channels(B_extra_cut='nRemainingTracksInEvent == 1', specific=True)
configuration = fei.config.FeiConfiguration(prefix='FEI_Belle2_Specific_2017_1', training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)

if feistate.stage in [0, 1]:
    feistate.path.add_module('RootOutput')
    roe_path = create_path()
    cond_module = register_module('SignalSideParticleFilter')
    cond_module.param('particleLists', ['B+:sig'])
    cond_module.if_true(feistate.path, AfterConditionPath.END)
    roe_path.add_module(cond_module)
    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
else:
    path.add_path(feistate.path)

if feistate.stage > 0:
    path.add_module("RemoveParticlesNotInLists", particleLists=feistate.plists + ['B+:sig', 'B+:FEIMC'])
    path.add_module('RootOutput')

print(path)
process(path)
