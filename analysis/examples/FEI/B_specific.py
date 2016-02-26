#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *

particles = get_default_channels(B_extra_cut='nRemainingTracksInEvent == 1')

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

# fillParticleList('gamma:sig', 'E > 0.03 and clusterE9E25 > 0.905 and clusterTiming < 50', writeOut=True, path=path)
# reconstructDecay('pi0:sig ->  gamma:sig gamma:sig', '0.116 < M < 0.154', writeOut=True, path=path)
# reconstructDecay('tau+:sig4 ->  pi+:sig pi0:sig', '0.570 < M < 0.970', 4, writeOut=True, path=path)

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

feistate = fullEventInterpretation('B+:sig', path, particles)

print(feistate.path)
process(feistate.path)
