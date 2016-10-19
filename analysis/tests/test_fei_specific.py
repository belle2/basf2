#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
import shutil
import glob
import sys

tempdir = tempfile.mkdtemp()
os.chdir(tempdir)

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2

from fei.default_channels import get_unittest_channels
from fei.steering import fullEventInterpretation
import fei.provider

fei.provider.MaximumNumberOfMVASamples = int(1e7)
fei.provider.MinimumNumberOfMVASamples = int(10)

filepath = 'analysis/tests/mdst6.root'
inputFile = Belle2.FileSystem.findFile(filepath)
if len(inputFile) == 0:
    sys.stderr.write(
        "TEST SKIPPED: input file " +
        filepath +
        " not found. You can retrieve it via 'wget http://www-ekp.physik.uni-karlsruhe.de/~tkeck/mdst6.root'\n")
    sys.exit(-1)

selection_path = create_path()
selection_path.add_module('RootInput', inputFileName=inputFile)
selection_path.add_module('Gearbox')
selection_path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])

fillParticleLists([('mu+:signal', 'muid > 0.1')], True, selection_path)
matchMCTruth('mu+:signal', path=selection_path)
reconstructDecay('tau+:signal ->  mu+:signal', '', writeOut=True, path=selection_path)
matchMCTruth('tau+:signal', path=selection_path)
reconstructDecay('B+:signal -> tau+:signal', '', writeOut=True, path=selection_path)
matchMCTruth('B+:signal', path=selection_path)
buildRestOfEvent('B+:signal', path=selection_path)

selection_path.add_module('MCDecayFinder', decayString='B+ ==> tau+ nu_tau', listName='B+:FEIMC', writeOut=True)

particles = get_unittest_channels()

sys.argv.append('-verbose')
sys.argv.append('-prune')

# MCCount Histograms,
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1

# Train FSP Networks
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 2
assert len(glob.glob('mu+*')) == 1
assert len(glob.glob('pi+*')) == 1
assert len(glob.glob('K+*')) == 1

# Train pi0 network and independent D networks
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 4  # V0 Gamma channel is not trained
assert len(glob.glob('mu+*')) == 3
assert len(glob.glob('pi+*')) == 3
assert len(glob.glob('K+*')) == 3
assert len(glob.glob('pi0*')) == 1
assert len(glob.glob('D*')) == 3

# Train D networks
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('pi0*')) == 3
assert len(glob.glob('D*')) == 11

feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('moduleStatistics_*')) == 1
assert len(glob.glob('D*')) == 15

feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
assert feistate.is_trained
assert len(glob.glob('Summary*.pickle')) == 1


sys.argv.append('-monitor')

feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
process(feistate.path)
assert feistate.is_trained
assert len(glob.glob('Monitor_MCCounts.root')) == 1
assert len(glob.glob('Monitor_ModuleStatistics.root')) == 1
assert len(glob.glob('Monitor_TagUniqueSignal_*')) == 7
assert len(glob.glob('Monitor_FitVertex_*')) == 12
assert len(glob.glob('Monitor_Final_*')) == 7
assert len(glob.glob('Monitor_CopyParticleList_*')) == 21
assert len(glob.glob('Monitor_MatchParticleList_*')) == 11
assert len(glob.glob('Monitor_SignalProbability_*')) == 10
assert len(glob.glob('Monitor_MakeParticleList_*')) == 22
assert len(glob.glob('Monitor_*')) == 92

sys.argv.append('-dump-path')
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
feistate.path.modules()[0].param('inputFileName', inputFile)
feistate.path.modules()[0].param('inputFileNames', [])
uncached_path = serialize_path(feistate.path)
sys.argv.pop()

shutil.rmtree(tempdir)

tempdir = tempfile.mkdtemp()
os.chdir(tempdir)

sys.argv.append('-cache')
sys.argv.append('cache.pickle')

# MCCount Histograms,
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1
assert len(glob.glob('cache.pickle')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

# Train FSP Networks
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 2
assert len(glob.glob('mu+*')) == 1
assert len(glob.glob('pi+*')) == 1
assert len(glob.glob('K+*')) == 1
assert len(glob.glob('cache.pickle.bkp0')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

# Train pi0 network and independent D networks
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('gamma*')) == 4  # V0 Gamma channel is not trained
assert len(glob.glob('mu+*')) == 3
assert len(glob.glob('pi+*')) == 3
assert len(glob.glob('K+*')) == 3
assert len(glob.glob('pi0*')) == 1
assert len(glob.glob('D*')) == 3
assert len(glob.glob('cache.pickle.bkp1')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

# Train D networks
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('pi0*')) == 3
assert len(glob.glob('D*')) == 11
assert len(glob.glob('cache.pickle.bkp2')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
process(feistate.path)
assert not feistate.is_trained
assert len(glob.glob('RootOutput.root')) == 1
assert len(glob.glob('moduleStatistics_*')) == 1
assert len(glob.glob('D*')) == 15
assert len(glob.glob('cache.pickle.bkp3')) == 1
shutil.copyfile('RootOutput.root', 'RootInput.root')

feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
feistate.path.modules()[0].param('inputFileName', 'RootInput.root')
assert len(glob.glob('Summary*.pickle')) == 1
assert len(glob.glob('cache.pickle.bkp4')) == 1
# Check if now all Monitoring histograms are available
assert len(glob.glob('Monitor_MCCounts.root')) == 1
assert len(glob.glob('Monitor_ModuleStatistics.root')) == 1
assert len(glob.glob('Monitor_TagUniqueSignal_*')) == 7
assert len(glob.glob('Monitor_FitVertex_*')) == 12
assert len(glob.glob('Monitor_Final_*')) == 7
assert len(glob.glob('Monitor_CopyParticleList_*')) == 21
assert len(glob.glob('Monitor_MatchParticleList_*')) == 11
assert len(glob.glob('Monitor_SignalProbability_*')) == 10
assert len(glob.glob('Monitor_MakeParticleList_*')) == 22
# Additional monitoring stuff during training
assert len(glob.glob('Monitor_GenerateTrainingData_*')) == 11
assert len(glob.glob('Monitor_*')) == 103
assert feistate.is_trained

sys.argv.pop()
sys.argv.pop()

sys.argv.append('-dump-path')
feistate = fullEventInterpretation('B+:signal', selection_path, particles, 'FEITEST')
feistate.path.modules()[0].param('inputFileName', inputFile)
feistate.path.modules()[0].param('inputFileNames', [])
cached_path = serialize_path(feistate.path)

print(cached_path)
print(uncached_path)
assert uncached_path == cached_path


shutil.rmtree(tempdir)
