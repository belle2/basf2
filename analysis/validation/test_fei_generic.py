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

path = create_path()
inputMdstList('MC6', [inputFile], path)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=-1, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 0
print(path)
process(path)
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1

fei.do_trainings(particles, configuration)

path = create_path()
inputMdstList('MC6', ['./RootOutput.root'], path)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 1
print(path)
process(path)
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

path = create_path()
inputMdstList('MC6', ['./RootOutput.root'], path)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 2
print(path)
process(path)
assert len(glob.glob('pi0*')) == 1

fei.do_trainings(particles, configuration)
assert len(glob.glob('pi0*')) == 3

path = create_path()
inputMdstList('MC6', ['./RootOutput.root'], path)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 4
print(path)
process(path)
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('D*')) == 5

# One training will fail D -> pi pi due to low statistic
fei.do_trainings(particles, configuration)
assert len(glob.glob('D*')) == 15

path = create_path()
inputMdstList('MC6', ['./RootOutput.root'], path)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 7

path = create_path()
inputMdstList('MC6', [inputFile], path)
configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=0, monitor=True)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 7
print(path)
process(path)
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
