#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>wsut@uni-bonn.de</contact>
</header>
"""

import fei
from fei.default_channels import get_unittest_channels
import os

import tempfile
import shutil
import glob

import basf2 as b2
import modularAnalysis as ma
import basf2_mva

basf2_mva.loadRootDictionary()

tempdir = tempfile.mkdtemp()
os.chdir(tempdir)

b2.conditions.append_testing_payloads('localdb/database.txt')

fei.core.Teacher.MaximumNumberOfMVASamples = int(1e7)
fei.core.Teacher.MinimumNumberOfMVASamples = int(10)

particles = fei.get_unittest_channels()

sig_path = b2.create_path()
ma.inputMdst(environmentType='default',
             filename=b2.find_file('mdst14.root', 'validation', False),
             path=sig_path)
ma.fillParticleList('mu+:sig', 'muonID > 0.5 and dr < 1 and abs(dz) < 2', writeOut=True, path=sig_path)
ma.reconstructDecay('tau+:sig -> mu+:sig', '', 1, writeOut=True, path=sig_path)
ma.matchMCTruth('tau+:sig', path=sig_path)
ma.reconstructDecay('B+:sig -> tau+:sig', '', writeOut=True, path=sig_path)
ma.matchMCTruth('B+:sig', path=sig_path)
ma.buildRestOfEvent('B+:sig', path=sig_path)

particles = get_unittest_channels(specific=True)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', training=True, cache=-1)
feistate = fei.get_path(particles, configuration)

path = b2.create_path()
roe_path = b2.create_path()
cond_module = ma.register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, b2.AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 0
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('mcParticlesCount.root')) == 1

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', training=True, cache=0)
fei.do_trainings(particles, configuration)
path = b2.create_path()
feistate = fei.get_path(particles, configuration)
roe_path = b2.create_path()
cond_module = ma.register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, b2.AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 1, feistate.stage
print(path)
b2.process(path, max_event=10000)
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
path = b2.create_path()
feistate = fei.get_path(particles, configuration)
roe_path = b2.create_path()
cond_module = ma.register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, b2.AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 2
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('pi0*')) == 1

fei.do_trainings(particles, configuration)
assert len(glob.glob('pi0*')) == 3

fei.do_trainings(particles, configuration)
path = b2.create_path()
feistate = fei.get_path(particles, configuration)
roe_path = b2.create_path()
cond_module = ma.register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, b2.AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 4
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('D*')) == 5

# One training will fail D -> pi pi due to low statistic
fei.do_trainings(particles, configuration)
assert len(glob.glob('D*')) == 15

fei.do_trainings(particles, configuration)
path = b2.create_path()
feistate = fei.get_path(particles, configuration)
roe_path = b2.create_path()
cond_module = ma.register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, b2.AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 7

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', monitor=True, training=False)
feistate = fei.get_path(particles, configuration)

fei.do_trainings(particles, configuration)
path = b2.create_path()
feistate = fei.get_path(particles, configuration)
roe_path = b2.create_path()
cond_module = ma.register_module('SignalSideParticleFilter')
cond_module.param('particleLists', ['B+:sig'])
cond_module.if_true(feistate.path, b2.AfterConditionPath.END)
roe_path.add_module(cond_module)
path.add_path(sig_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

assert feistate.stage == 7
print(path)
b2.process(path, max_event=10000)
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
