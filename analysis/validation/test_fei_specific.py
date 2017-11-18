#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Thomas Keck; thomas.keck2@kit.edu</contact>
</header>
"""

import os
import sys

# if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
#     sys.exit(0)

import tempfile
import shutil
import glob
import sys

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2
import basf2_mva
import pdg

basf2_mva.loadRootDictionary()

tempdir = tempfile.mkdtemp()
os.chdir(tempdir)

use_local_database(tempdir + '/localdb/dbcache.txt', tempdir + '/localdb/', False, LogLevel.WARNING)

from fei.default_channels import get_unittest_channels
import fei

fei.core.Teacher.MaximumNumberOfMVASamples = int(1e7)
fei.core.Teacher.MinimumNumberOfMVASamples = int(10)

particles = fei.get_unittest_channels()

if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
    sys.exit(0)

# inputFile = '/storage/jbod/tkeck/MC6/evtgen-charged/sub00/mdst_000020_prod00000189_task00000020.root'
inputFile = os.path.join(os.environ['BELLE2_VALIDATION_DATA_DIR'], 'analysis/mdst6_BBx0_charged.root')

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

sig_path = create_path()
inputMdstList('MC6', [inputFile], sig_path)
fillParticleList('mu+:sig', 'muonID > 0.5 and dr < 1 and abs(dz) < 2', writeOut=True, path=sig_path)
reconstructDecay('tau+:sig -> mu+:sig', '', 1, writeOut=True, path=sig_path)
matchMCTruth('tau+:sig', path=sig_path)
reconstructDecay('B+:sig -> tau+:sig', '', writeOut=True, path=sig_path)
matchMCTruth('B+:sig', path=sig_path)
buildRestOfEvent('B+:sig', path=sig_path)

particles = get_unittest_channels(specific=True)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', training=True, cache=-1)
feistate = fei.get_path(particles, configuration)

path = create_path()
roe_path = create_path()
cond_module = register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 0
print(path)
process(path)
assert len(glob.glob('mcParticlesCount.root')) == 1

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', training=True, cache=0)
fei.do_trainings(particles, configuration)
path = create_path()
feistate = fei.get_path(particles, configuration)
roe_path = create_path()
cond_module = register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 1, feistate.stage
print(path)
process(path)
assert len(glob.glob('gamma*')) == 2
assert len(glob.glob('mu+*')) == 1
assert len(glob.glob('pi+*')) == 1
assert len(glob.glob('K+*')) == 1

fei.do_trainings(particles, configuration)

assert len(glob.glob('gamma*')) == 6
assert len(glob.glob('mu+*')) == 3
assert len(glob.glob('pi+*')) == 3
assert len(glob.glob('K+*')) == 3

fei.do_trainings(particles, configuration)
path = create_path()
feistate = fei.get_path(particles, configuration)
roe_path = create_path()
cond_module = register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 2
print(path)
process(path)
assert len(glob.glob('pi0*')) == 1

fei.do_trainings(particles, configuration)
assert len(glob.glob('pi0*')) == 3

fei.do_trainings(particles, configuration)
path = create_path()
feistate = fei.get_path(particles, configuration)
roe_path = create_path()
cond_module = register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 4
print(path)
process(path)
assert len(glob.glob('D*')) == 5

# One training will fail D -> pi pi due to low statistic
fei.do_trainings(particles, configuration)
assert len(glob.glob('D*')) == 15

fei.do_trainings(particles, configuration)
path = create_path()
feistate = fei.get_path(particles, configuration)
roe_path = create_path()
cond_module = register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 7

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', monitor=True, training=False)
feistate = fei.get_path(particles, configuration)

fei.do_trainings(particles, configuration)
path = create_path()
feistate = fei.get_path(particles, configuration)
roe_path = create_path()
cond_module = register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 7
print(path)
process(path)
assert len(glob.glob('Monitor_FSPLoader.root')) == 1
assert len(glob.glob('Monitor_TrainingData_*')) == 11
assert len(glob.glob('Monitor_PreReconstruction_BeforeRanking_*')) == 11
assert len(glob.glob('Monitor_PreReconstruction_AfterRanking_*')) == 11
assert len(glob.glob('Monitor_PreReconstruction_AfterVertex_*')) == 11
assert len(glob.glob('Monitor_PostReconstruction_AfterMVA_*')) == 6
assert len(glob.glob('Monitor_PostReconstruction_BeforePostCut_*')) == 5
assert len(glob.glob('Monitor_PostReconstruction_BeforeRanking_*')) == 5
assert len(glob.glob('Monitor_PostReconstruction_AfterRanking_*')) == 5
assert len(glob.glob('Monitor_Final_*')) == 5
assert len(glob.glob('Monitor_ModuleStatistics.root')) == 1

shutil.rmtree(tempdir)
