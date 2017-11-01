#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters

path = create_path()
path.add_module('RootInput')

path.add_module('Gearbox')
path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])
beamparameters = add_beamparameters(analysis_main, 'Y4S')

fillParticleList('mu+', 'muonID > 0.5', path=path)
reconstructDecay('tau+ ->  mu+', '', path=path)
matchMCTruth('tau+', path=path)
reconstructDecay('B+:sig -> tau+', '', path=path)
matchMCTruth('B+:sig', path=path)
buildRestOfEvent('B+:sig', path=path)

roe_path = create_path()
roe_path.add_module('DeepFEIExtractor', filename='data.dpf')

path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
path.add_module('ProgressBar')
process(path)
