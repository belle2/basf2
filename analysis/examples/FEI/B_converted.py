#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

from basf2 import *
from modularAnalysis import *

import b2biiConversion
import ROOT
from ROOT import Belle2
ROOT.Belle2.BFieldManager.getInstance().setConstantOverride(0, 0, 1.5 * ROOT.Belle2.Unit.T)

import fei
particles = fei.get_default_channels(convertedFromBelle=True)
configuration = fei.config.FeiConfiguration(prefix='FEI_Belle1_Generic_2017_1', b2bii=True, training=True)
feistate = fei.get_path(particles, configuration)

path = create_path()
if feistate.stage <= 0:
    b2biiConversion.convertBelleMdstToBelleIIMdst(None, applyHadronBJSkim=True, path=path)
    setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)
else:
    inputMdstList('Belle', [], path)

path.add_path(feistate.path)

if feistate.stage >= 0:
    path.add_module("RemoveParticlesNotInLists", particleLists=feistate.plists)
    path.add_module('RootOutput')

print(path)
process(path)
