#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>wsut@uni-bonn.de</contact>
</header>
"""

import fei
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

path = b2.create_path()

ma.inputMdst(environmentType='default',
             filename=b2.find_file('mdst14.root', 'validation', False),
             path=path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=-1, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 0
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1

fei.do_trainings(particles, configuration)

path = b2.create_path()
ma.inputMdstList('default', ['./RootOutput.root'], path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 1
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 2
assert len(glob.glob('mu+*')) == 1
assert len(glob.glob('pi+*')) == 1
assert len(glob.glob('K+*')) == 1

fei.do_trainings(particles, configuration)

assert len(glob.glob('gamma*')) == 6
assert len(glob.glob('mu+*')) == 3
assert len(glob.glob('pi+*')) == 3
assert len(glob.glob('K+*')) == 3

path = b2.create_path()
ma.inputMdstList('default', ['./RootOutput.root'], path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 2
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('pi0*')) == 1

fei.do_trainings(particles, configuration)
assert len(glob.glob('pi0*')) == 3

path = b2.create_path()
ma.inputMdstList('default', ['./RootOutput.root'], path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 4
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('D*')) == 5

# One training will fail D -> pi pi due to low statistic
fei.do_trainings(particles, configuration)
print(len(glob.glob('D*')))
assert len(glob.glob('D*')) == 15

path = b2.create_path()
ma.inputMdstList('default', ['./RootOutput.root'], path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 7

path = b2.create_path()
ma.inputMdst(environmentType='default',
             filename=b2.find_file('mdst14.root', 'validation', False),
             path=path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=0, monitor=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 7
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('Monitor_FSPLoader.root')) == 1
assert len(glob.glob('Monitor_TrainingData_*')) == 11
assert len(glob.glob('Monitor_PreReconstruction_BeforeRanking_*')) == 11
assert len(glob.glob('Monitor_PreReconstruction_AfterRanking_*')) == 11
assert len(glob.glob('Monitor_PreReconstruction_AfterVertex_*')) == 11
assert len(glob.glob('Monitor_PostReconstruction_AfterMVA_*')) == 11
assert len(glob.glob('Monitor_PostReconstruction_BeforePostCut_*')) == 7
assert len(glob.glob('Monitor_PostReconstruction_BeforeRanking_*')) == 7
assert len(glob.glob('Monitor_PostReconstruction_AfterRanking_*')) == 7
assert len(glob.glob('Monitor_Final_*')) == 7
assert len(glob.glob('Monitor_ModuleStatistics.root')) == 1

shutil.rmtree(tempdir)
