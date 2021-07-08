#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>wsut@uni-bonn.de</contact>
</header>
"""

import os
import subprocess

import tempfile
import shutil
import glob

import fei
import basf2 as b2
import modularAnalysis as ma
import basf2_mva
import ROOT

basf2_mva.loadRootDictionary()

tempdir = tempfile.mkdtemp()
os.chdir(tempdir)

b2.conditions.append_testing_payloads('localdb/database.txt')

fei.core.Teacher.MaximumNumberOfMVASamples = int(1e7)
fei.core.Teacher.MinimumNumberOfMVASamples = int(10)

particles = fei.get_unittest_channels()

# Construct path for production of mcParticlesCount.root at stage -1
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

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=-1, training=True, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 0  # corresponds to stage -1, since increased by 1 after creating path
print(path)
b2.process(path, max_event=10000)
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1

# Construct path for production of stage 0 training data
path = b2.create_path()
ma.inputMdstList('default', ['./RootOutput.root'], path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 1  # corresponds to stage 0, since increased by 1 after creating path
print(path)
b2.process(path, max_event=10000)

# Check avaliability of training input and cache file for stage 0
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('training_input.root')) == 1
f = ROOT.TFile.Open("training_input.root", "read")
assert sum(['gamma' in key.GetName()[:5] for key in f.GetListOfKeys()]) == 2
assert sum(['mu+' in key.GetName()[:4] for key in f.GetListOfKeys()]) == 1
assert sum(['pi+' in key.GetName()[:4] for key in f.GetListOfKeys()]) == 1
assert sum(['K+' in key.GetName()[:3] for key in f.GetListOfKeys()]) == 1
f.Close()

# Perform stage 0 training
fei.do_trainings(particles, configuration)

# Moving training_input.root to training_input_stage0.root for later usage
shutil.move("training_input.root", f"training_input_stage{feistate.stage-1}.root")

# Check availability of stage 0 *.xml training files
assert len(glob.glob('gamma*.xml')) == 2
assert len(glob.glob('mu+*.xml')) == 1
assert len(glob.glob('pi+*.xml')) == 1
assert len(glob.glob('K+*.xml')) == 1

# Construct path for production of stage 1 training data
path = b2.create_path()
ma.inputMdstList('default', ['./RootOutput.root'], path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 2  # corresponds to stage 1, since increased by 1 after creating path
print(path)
b2.process(path, max_event=10000)

# Check avaliability of training input and cache file for stage 1
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('training_input.root')) == 1
f = ROOT.TFile.Open("training_input.root", "read")
assert sum(['pi0' in key.GetName()[:4] for key in f.GetListOfKeys()]) == 1
f.Close()

# Perform stage 1 training
fei.do_trainings(particles, configuration)

# Moving training_input.root to training_input_stage1.root for later usage
shutil.move("training_input.root", f"training_input_stage{feistate.stage-1}.root")

# Check availability of stage 1 *.xml training files
assert len(glob.glob('pi0*.xml')) == 1

# Construct path for production of stage 3 training data (stage 2 is skipped)
path = b2.create_path()
ma.inputMdstList('default', ['./RootOutput.root'], path)

maxTracks = 12
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>{}'.format(maxTracks), empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

configuration = fei.config.FeiConfiguration(prefix='FEI_VALIDATION', cache=feistate.stage, training=True, monitor=False)
feistate = fei.get_path(particles, configuration)
path.add_path(feistate.path)
path.add_module('RootOutput')

assert feistate.stage == 4  # corresponds to stage 3, since increased by 1 after creating path
print(path)
b2.process(path, max_event=10000)

# Check avaliability of training input and cache file for stage 3
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('training_input.root')) == 1
f = ROOT.TFile.Open("training_input.root", "read")
assert sum(['D' in key.GetName()[:2] for key in f.GetListOfKeys()]) == 5
f.Close()

# Perform stage 3 training
fei.do_trainings(particles, configuration)

# Moving training_input.root to training_input_stage3.root for later usage
shutil.move("training_input.root", f"training_input_stage{feistate.stage-1}.root")

# Check availability of stage 3 *.xml training files
assert len(glob.glob('D*.xml')) == 5

# Merge training input files for validation
subprocess.call(["analysis-fei-mergefiles", "training_input.root"] + glob.glob("training_input_stage*.root"))

# Expect 4 different training_input*.root files now
assert len(glob.glob('training_input*.root')) == 4

# Construct path for stage 6 preparing evaluation (stages 4 and 5 skipped, input evaluated from stage 0 on)
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

assert feistate.stage == 7  # corresponds to stage 6, since increased by 1 after creating path
print(path)
b2.process(path, max_event=10000)

# Check avaliability of monitoring files constructed at stage 6 for validation
assert len(glob.glob('Monitor_FSPLoader.root')) == 1

assert len(glob.glob('Monitor_ModuleStatistics.root')) == 1

assert len(glob.glob('Monitor_Final.root')) == 1
f = ROOT.TFile.Open("Monitor_Final.root", "read")
assert len([key.GetName() for key in f.GetListOfKeys()]) == 7
f.Close()

prereconstruction_files = [
    'Monitor_PreReconstruction_BeforeRanking.root',
    'Monitor_PreReconstruction_AfterRanking.root',
    'Monitor_PreReconstruction_AfterVertex.root',
    'Monitor_PostReconstruction_AfterMVA.root'
]

for fname in prereconstruction_files:
    assert len(glob.glob(fname)) == 1
    f = ROOT.TFile.Open(fname, "read")
    assert len([key.GetName() for key in f.GetListOfKeys()]) == 11
    f.Close()

postreconstruction_files = [
    'Monitor_PostReconstruction_BeforePostCut.root',
    'Monitor_PostReconstruction_BeforeRanking.root',
    'Monitor_PostReconstruction_AfterRanking.root'
]

for fname in postreconstruction_files:
    assert len(glob.glob(fname)) == 1
    f = ROOT.TFile.Open(fname, "read")
    assert len([key.GetName() for key in f.GetListOfKeys()]) == 7
    f.Close()

shutil.rmtree(tempdir)
