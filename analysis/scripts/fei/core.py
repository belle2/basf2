#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# @cond

# Thomas Keck 2017

"""
 The Full Event Interpretation Algorithm

 Some basic facts:
  - The algorithm will automatically reconstruct B mesons and calculate a signal probability for each candidate.
  - It can be used for hadronic and semileptonic tagging.
  - The algorithm has to be trained on MC, and can afterwards be applied on data.
  - The training requires O(100) million MC events
  - The weightfiles are stored in the Belle 2 Condition database

 Read this file if you want to understand the technical details of the FEI.

 The FEI follows a hierarchical approach.
 There are 7 stages:
   (Stage -1: Write out information about the provided data sample)
   Stage 0: Final State Particles (FSP)
   Stage 1: pi0, J/Psi
   Stage 2: K_S0
   Stage 3: D mesons
   Stage 4: D* mesons
   Stage 5: B mesons
   Stage 6: Finish

 Most stages consists of:
   - Create Particle Candidates
   - Apply Cuts
   - Do vertex Fitting
   - Apply a multivariate classification method
   - Apply more Cuts

 The FEI will reconstruct these 7 stages during the training phase,
 since the stages depend on one another, you have to run basf2 multiple (7) times on the same data
 to train all the necssary multivariate classifiers.
"""

# FEI defines own command line options, therefore we disable
# the ROOT command line options, which otherwise interfere sometimes.
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True

# FEI uses multi-threading for parallel execution of tasks therefore
# the ROOT gui-thread is disabled, which otherwise interferes sometimes
PyConfig.StartGuiThread = False
import ROOT
from ROOT import Belle2

# Import basf2
import basf2
import pybasf2
from basf2 import *
from modularAnalysis import *

import basf2_mva

# Should come after basf2 import
import pdg

from fei import config

# Standard python modules
import collections
import argparse
import os
import shutil
import typing
import pickle
import re
import functools
import subprocess
import multiprocessing
import pickle

# Simple object containing the output of fei
FeiState = collections.namedtuple('FeiState', 'path, stage, plists')


class TrainingDataInformation(object):
    """
    Contains the relevant information about the used training data.
    Basically we write out the number of MC particles in the whole dataset.
    This numbers we can use to calculate what fraction of candidates we have to write
    out as TrainingData to get a reasonable amount of candidates to train on
    (too few candidates will lead to heavy overtraining, to many won't fit into memory).
    Secondly we can use this information for the generation of the monitoring pdfs,
    where we calculate reconstruction efficiencies.
    """
    def __init__(self, particles: typing.Sequence[config.Particle]):
        """
        Create a new TrainingData object
        @param particles list of config.Particle objects
        """
        #: list of config.Particle objects
        self.particles = particles
        #: filename
        self.filename = 'mcParticlesCount.root'

    def available(self) -> bool:
        """
        Check if the relevant information is already available
        """
        return os.path.isfile(self.filename)

    def reconstruct(self) -> pybasf2.Path:
        """
        Returns pybasf2.Path which counts the number of MCParticles in each event.
        @param particles list of config.Particle objects
        """
        # Unique absolute pdg-codes of all particles
        pdgs = set([abs(pdg.from_name(particle.name)) for particle in self.particles])

        path = basf2.create_path()
        module = register_module('VariablesToHistogram')
        module.set_name("VariablesToHistogram_MCCount")
        module.param('variables', [('NumberOfMCParticlesInEvent({i})'.format(i=pdg), 100, -0.5, 99.5) for pdg in pdgs])
        module.param('fileName', self.filename)
        path.add_module(module)
        return path

    def get_mc_counts(self):
        """
        Read out the number of MC particles from the file created by reconstruct
        """
        # Unique absolute pdg-codes of all particles
        root_file = ROOT.TFile(self.filename)
        mc_counts = {}

        Belle2.Variable.Manager

        for key in root_file.GetListOfKeys():
            variable = Belle2.invertMakeROOTCompatible(key.GetName())
            pdg = abs(int(variable[len('NumberOfMCParticlesInEvent('):-len(")")]))
            hist = key.ReadObj()
            mc_counts[pdg] = {}
            mc_counts[pdg]['sum'] = sum(hist.GetXaxis().GetBinCenter(bin + 1) * hist.GetBinContent(bin + 1)
                                        for bin in range(hist.GetNbinsX()))
            mc_counts[pdg]['std'] = hist.GetStdDev()
            mc_counts[pdg]['avg'] = hist.GetMean()
            mc_counts[pdg]['max'] = hist.GetXaxis().GetBinCenter(hist.FindLastBinAbove(0.0))
            mc_counts[pdg]['min'] = hist.GetXaxis().GetBinCenter(hist.FindFirstBinAbove(0.0))

        mc_counts[0] = {}
        mc_counts[0]['sum'] = hist.GetEntries()
        return mc_counts


class FSPLoader(object):
    """
    Steers the loading of FSP particles.
    This does NOT include RootInput, Geometry or anything required before loading FSPs,
    the user has to add this himself (because it depends on the MC campaign and if you want
    to use Belle 1 or Belle 2).
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration):
        """
        Create a new FSPLoader object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        """
        #: list of config.Particle objects
        self.particles = particles
        #: config.FeiConfiguration object
        self.config = config

    def reconstruct(self) -> pybasf2.Path:
        """
        Returns pybasf2.Path which loads the FSP Particles
        """
        path = basf2.create_path()

        if self.config.b2bii:
            fillParticleLists([('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                               ('mu+:FSP', ''), ('p+:FSP', ''), ('K_L0:FSP', '')], writeOut=True, path=path)
            for outputList, inputList in [('gamma:FSP', 'gamma:mdst'), ('K_S0:V0', 'K_S0:mdst'),
                                          ('pi0:FSP', 'pi0:mdst'), ('gamma:V0', 'gamma:v0mdst')]:
                copyParticles(outputList, inputList, writeOut=True, path=path)
        else:
            fillParticleLists([('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                               ('mu+:FSP', ''), ('gamma:FSP', ''), ('K_S0:V0', ''),
                               ('p+:FSP', ''), ('K_L0:FSP', ''), ('Lambda0:FSP', '')], writeOut=True, path=path)
            fillConvertedPhotonsList('gamma:V0', '', writeOut=True, path=path)

        if self.config.monitor:
            names = ['e+', 'K+', 'pi+', 'mu+', 'gamma', 'K_S0', 'p+', 'K_L0', 'Lambda0', 'pi0']
            filename = 'Monitor_FSPLoader.root'
            pdgs = set([abs(pdg.from_name(name)) for name in names])
            variables = [('NumberOfMCParticlesInEvent({i})'.format(i=pdg), 100, -0.5, 99.5) for pdg in pdgs]
            variablesToHistogram('', variables=variables, filename=filename, path=path)
        return path


class TrainingData(object):
    """
    Steers the creation of the training data.
    The training data is used to train a multivariate classifier for each channel.
    The training of the FEI at its core is just generating this training data for each channel.
    After we created the training data for a stage, we have to train the classifiers (see Teacher class further down).
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration,
                 mc_counts: typing.Mapping[int, typing.Mapping[str, float]]):
        """
        Create a new TrainingData object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        @param mc_counts containing number of MC Particles
        """
        #: list of config.Particle objects
        self.particles = particles
        #: config.FeiConfiguration object
        self.config = config
        #: containing number of MC Particles
        self.mc_counts = mc_counts

    def reconstruct(self) -> pybasf2.Path:
        """
        Returns pybasf2.Path which creates the training data for the given particles
        """
        path = basf2.create_path()

        for particle in self.particles:
            pdgcode = abs(pdg.from_name(particle.name))
            nSignal = self.mc_counts[pdgcode]['sum']
            # For D-Mesons we usually have a efficiency of 10^-3 including branching fraction
            if pdgcode > 400:
                nSignal /= 1000
            # For B-Mesons we usually have a efficiency of 10^-4 including branching fraction
            if pdgcode > 500:
                nSignal /= 10000

            for channel in particle.channels:
                filename = '{}.root'.format(channel.label)

                nBackground = self.mc_counts[0]['sum'] * channel.preCutConfig.bestCandidateCut
                inverseSamplingRates = {}
                if nBackground > Teacher.MaximumNumberOfMVASamples:
                    inverseSamplingRates[0] = int(nBackground / Teacher.MaximumNumberOfMVASamples) + 1
                if nSignal > Teacher.MaximumNumberOfMVASamples:
                    inverseSamplingRates[1] = int(nSignal / Teacher.MaximumNumberOfMVASamples) + 1

                spectators = [channel.mvaConfig.target]
                if channel.mvaConfig.sPlotVariable is not None:
                    spectators.append(channel.mvaConfig.sPlotVariable)

                if self.config.monitor:
                    hist_variables = ['mcErrors', 'mcParticleStatus'] + channel.mvaConfig.variables + spectators
                    hist_variables_2d = [(x, channel.mvaConfig.target)
                                         for x in channel.mvaConfig.variables + spectators if x is not channel.mvaConfig.target]
                    hist_filename = 'Monitor_TrainingData_{}.root'.format(channel.label)
                    variablesToHistogram(channel.name,
                                         variables=config.variables2binnings(hist_variables),
                                         variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                         filename=config.removeJPsiSlash(hist_filename), path=path)

                teacher = register_module('VariablesToNtuple')
                teacher.set_name('VariablesToNtuple_' + channel.name)
                teacher.param('fileName', filename)
                teacher.param('treeName', 'variables')
                teacher.param('variables', channel.mvaConfig.variables + spectators)
                teacher.param('particleList', channel.name)
                teacher.param('sampling', (channel.mvaConfig.target, inverseSamplingRates))
                path.add_module(teacher)
        return path


class PreReconstruction(object):
    """
    Steers the reconstruction phase before the mva method was applied
    It Includes:
        - The ParticleCombination (for each particle and channel we create candidates using
                                   the daughter candidates from the previous stages)
        - MC Matching
        - Vertex Fitting (this is the slowest part of the whole FEI, KFit is used by default,
                          but you can use fastFit as a drop-in replacement https://github.com/thomaskeck/FastFit/,
                          this will speed up the whole FEI by a factor 2-3)
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration):
        """
        Create a new PreReconstruction object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        """
        #: list of config.Particle objects
        self.particles = particles
        #: config.FeiConfiguration object
        self.config = config

    def reconstruct(self) -> pybasf2.Path:
        """
        Returns pybasf2.Path which reconstructs the particles and does the vertex fitting if necessary
        """
        path = basf2.create_path()

        for particle in self.particles:
            for channel in particle.channels:

                if len(channel.daughters) == 1:
                    cutAndCopyList(channel.name, channel.daughters[0], channel.preCutConfig.userCut, writeOut=True, path=path)
                    variablesToExtraInfo(channel.name, {'constant({})'.format(channel.decayModeID): 'decayModeID'}, path=path)
                else:
                    reconstructDecay(channel.decayString, channel.preCutConfig.userCut, channel.decayModeID,
                                     writeOut=True, path=path)
                if self.config.monitor:
                    matchMCTruth(channel.name, path=path)
                    bc_variable = channel.preCutConfig.bestCandidateVariable
                    hist_variables = [bc_variable, 'mcErrors', 'mcParticleStatus', channel.mvaConfig.target]
                    hist_variables_2d = [(bc_variable, channel.mvaConfig.target),
                                         (bc_variable, 'mcErrors'),
                                         (bc_variable, 'mcParticleStatus')]
                    filename = 'Monitor_PreReconstruction_BeforeRanking_{}.root'.format(channel.label)
                    variablesToHistogram(channel.name,
                                         variables=config.variables2binnings(hist_variables),
                                         variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                         filename=filename, path=path)

                if channel.preCutConfig.bestCandidateMode == 'lowest':
                    rankByLowest(channel.name,
                                 channel.preCutConfig.bestCandidateVariable,
                                 channel.preCutConfig.bestCandidateCut,
                                 'preCut_rank',
                                 path=path)
                elif channel.preCutConfig.bestCandidateMode == 'highest':
                    rankByHighest(channel.name,
                                  channel.preCutConfig.bestCandidateVariable,
                                  channel.preCutConfig.bestCandidateCut,
                                  'preCut_rank',
                                  path=path)
                else:
                    raise RuntimeError("Unkown bestCandidateMode " + repr(channel.preCutConfig.bestCandidateMode))

                if self.config.monitor:
                    filename = 'Monitor_PreReconstruction_AfterRanking_{}.root'.format(channel.label)
                    hist_variables += ['extraInfo(preCut_rank)']
                    hist_variables_2d += [('extraInfo(preCut_rank)', channel.mvaConfig.target),
                                          ('extraInfo(preCut_rank)', 'mcErrors'),
                                          ('extraInfo(preCut_rank)', 'mcParticleStatus')]
                    variablesToHistogram(channel.name,
                                         variables=config.variables2binnings(hist_variables),
                                         variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                         filename=filename, path=path)
                # If we are not in monitor mode we do the mc matching now,
                # otherwise we did it above already!
                else:
                    matchMCTruth(channel.name, path=path)

                if re.findall(r"[\w']+", channel.decayString).count('pi0') > 1:
                    B2INFO("Ignoring vertex fit because multiple pi0 are not supported yet {c}.".format(c=channel.name))
                elif len(channel.daughters) > 1:
                    pvfit = register_module('ParticleVertexFitter')
                    pvfit.set_name('ParticleVertexFitter_' + channel.name)
                    pvfit.param('listName', channel.name)
                    pvfit.param('confidenceLevel', channel.preCutConfig.vertexCut)
                    pvfit.param('vertexFitter', 'kfitter')
                    pvfit.param('fitType', 'vertex')
                    pvfit.set_log_level(logging.log_level.ERROR)  # let's not produce gigabytes of uninteresting warnings
                    path.add_module(pvfit)

                if self.config.monitor:
                    hist_variables = ['chiProb', 'mcErrors', 'mcParticleStatus', channel.mvaConfig.target]
                    hist_variables_2d = [('chiProb', channel.mvaConfig.target),
                                         ('chiProb', 'mcErrors'),
                                         ('chiProb', 'mcParticleStatus')]
                    filename = 'Monitor_PreReconstruction_AfterVertex_{}.root'.format(channel.label)
                    variablesToHistogram(channel.name,
                                         variables=config.variables2binnings(hist_variables),
                                         variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                         filename=filename, path=path)

        return path


class PostReconstruction(object):
    """
    Steers the reconstruction phase after the mva method was applied
    It Includes:
        - The application of the mva method itself.
        - Copying all channel lists in a common one for each particle defined in particles
        - Tag unique signal candidates, to avoid double counting of channels with overlap
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration):
        """
        Create a new PostReconstruction object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        """
        #: list of config.Particle objects
        self.particles = particles
        #: config.FeiConfiguration object
        self.config = config

    def get_missing_channels(self) -> typing.Sequence[str]:
        """
        Returns all channels for which the weightfile is missing
        """
        missing = []
        for particle in self.particles:
            for channel in particle.channels:
                # weightfile = self.config.prefix + '_' + channel.label
                weightfile = channel.label + '.xml'
                if not basf2_mva.available(weightfile):
                    missing += [channel.label]
        return missing

    def available(self) -> bool:
        """
        Check if the relevant information is already available
        """
        return len(self.get_missing_channels()) == 0

    def reconstruct(self) -> pybasf2.Path:
        """
        Returns pybasf2.Path which reconstructs the particles and does the vertex fitting if necessary
        """
        path = basf2.create_path()

        for particle in self.particles:
            for channel in particle.channels:
                expert = register_module('MVAExpert')
                expert.set_name('MVAExpert_' + channel.name)
                if self.config.training:
                    expert.param('identifier', channel.label + '.xml')
                else:
                    expert.param('identifier', self.config.prefix + '_' + channel.label)
                expert.param('extraInfoName', 'SignalProbability')
                expert.param('listNames', [channel.name])
                path.add_module(expert)

                uniqueSignal = register_module('TagUniqueSignal')
                uniqueSignal.param('particleList', channel.name)
                uniqueSignal.param('target', channel.mvaConfig.target)
                uniqueSignal.param('extraInfoName', 'uniqueSignal')
                uniqueSignal.set_name('TagUniqueSignal_' + channel.name)
                path.add_module(uniqueSignal)

                if self.config.monitor:
                    hist_variables = ['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)', 'extraInfo(SignalProbability)',
                                      channel.mvaConfig.target, 'extraInfo(decayModeID)']
                    hist_variables_2d = [('extraInfo(SignalProbability)', channel.mvaConfig.target),
                                         ('extraInfo(SignalProbability)', 'mcErrors'),
                                         ('extraInfo(SignalProbability)', 'mcParticleStatus'),
                                         ('extraInfo(decayModeID)', channel.mvaConfig.target),
                                         ('extraInfo(decayModeID)', 'mcErrors'),
                                         ('extraInfo(decayModeID)', 'extraInfo(uniqueSignal)'),
                                         ('extraInfo(decayModeID)', 'mcParticleStatus')]
                    filename = 'Monitor_PostReconstruction_AfterMVA_{}.root'.format(channel.label)
                    variablesToHistogram(channel.name,
                                         variables=config.variables2binnings(hist_variables),
                                         variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                         filename=filename, path=path)

            cutstring = ''
            if particle.postCutConfig.value > 0.0:
                cutstring = str(particle.postCutConfig.value) + ' < extraInfo(SignalProbability)'

            copyLists(particle.identifier, [c.name for c in particle.channels], writeOut=True, path=path)

            if self.config.monitor:
                hist_variables = ['mcErrors', 'mcParticleStatus', 'extraInfo(uniqueSignal)', 'extraInfo(SignalProbability)',
                                  particle.mvaConfig.target, 'extraInfo(decayModeID)']
                hist_variables_2d = [('extraInfo(decayModeID)', particle.mvaConfig.target),
                                     ('extraInfo(decayModeID)', 'mcErrors'),
                                     ('extraInfo(decayModeID)', 'mcParticleStatus')]
                filename = 'Monitor_PostReconstruction_BeforePostCut_{}.root'.format(particle.identifier)
                variablesToHistogram(particle.identifier,
                                     variables=config.variables2binnings(hist_variables),
                                     variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                     filename=config.removeJPsiSlash(filename), path=path)

            applyCuts(particle.identifier, cutstring, path=path)

            if self.config.monitor:
                filename = 'Monitor_PostReconstruction_BeforeRanking_{}.root'.format(particle.identifier)
                variablesToHistogram(particle.identifier,
                                     variables=config.variables2binnings(hist_variables),
                                     variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                     filename=config.removeJPsiSlash(filename), path=path)

            rankByHighest(particle.identifier, 'extraInfo(SignalProbability)',
                          particle.postCutConfig.bestCandidateCut, 'postCut_rank', path=path)

            if self.config.monitor:
                hist_variables += ['extraInfo(postCut_rank)']
                hist_variables_2d += [('extraInfo(decayModeID)', 'extraInfo(postCut_rank)'),
                                      (particle.mvaConfig.target, 'extraInfo(postCut_rank)'),
                                      ('mcErrors', 'extraInfo(postCut_rank)'),
                                      ('mcParticleStatus', 'extraInfo(postCut_rank)')]
                filename = 'Monitor_PostReconstruction_AfterRanking_{}.root'.format(particle.identifier)
                variablesToHistogram(particle.identifier,
                                     variables=config.variables2binnings(hist_variables),
                                     variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                     filename=config.removeJPsiSlash(filename), path=path)

                variables = ['extraInfo(SignalProbability)', 'Mbc', 'mcErrors', 'mcParticleStatus', particle.mvaConfig.target,
                             'cosThetaBetweenParticleAndTrueB', 'extraInfo(uniqueSignal)', 'extraInfo(decayModeID)']
                filename = 'Monitor_Final_{}.root'.format(particle.identifier)
                variablesToNtuple(particle.identifier, variables, treename='variables',
                                  filename=config.removeJPsiSlash(filename), path=path)

        return path


class Teacher(object):
    """
    Performs all necessary trainings for all training data files which are available but where there is no weightfile avaiable yet.
    This class is usually used by the do_trainings function below, to perform the necessary trainings after each stage.
    The trainings are run in parallel using multi-threading of python.
    Each training is done by a subprocess call, the training command (passed by config.externTeacher) can be either
      * basf2_mva_teacher, the training will be done directly on the machine
      * externClustTeacher, the training will be submitted to the batch system of KEKCC
    """
    #: Maximum number of events per class,
    #: the sampling rates are chosen so that the training data does not exceed this number
    MaximumNumberOfMVASamples = int(1e7)
    #: Minimum number of events per class
    #: if the training data contains less events the channel is not used due to low statistics
    MinimumNumberOfMVASamples = int(5e2)

    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration):
        """
        Create a new Teacher object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        """
        #: list of config.Particle objects
        self.particles = particles
        #: config.FeiConfiguration object
        self.config = config

    @staticmethod
    def create_fake_weightfile(channel: str):
        """
        Create a fake weightfile using the trivial method, it will always return 0.0
        @param channel for which we create a fake weightfile
        """
        content = """
            <?xml version="1.0" encoding="utf-8"?>
            <method>Trivial</method>
            <weightfile>{channel}.xml</weightfile>
            <treename>tree</treename>
            <target_variable>isSignal</target_variable>
            <weight_variable>__weight__</weight_variable>
            <signal_class>1</signal_class>
            <max_events>0</max_events>
            <number_feature_variables>1</number_feature_variables>
            <variable0>M</variable0>
            <number_spectator_variables>0</number_spectator_variables>
            <number_data_files>1</number_data_files>
            <datafile0>train.root</datafile0>
            <Trivial_version>1</Trivial_version>
            <Trivial_output>0</Trivial_output>
            <signal_fraction>0.066082567</signal_fraction>
            """.format(channel=channel)
        with open(channel + ".xml", "w") as f:
            f.write(content)

    @staticmethod
    def check_if_weightfile_is_fake(filename: str):
        """
        Checks if the provided filename is a fake-weightfile or not
        @param filename the filename of the weightfile
        """
        try:
            return '<method>Trivial</method>' in open(filename, 'r').readlines()[2]
        except:
            return True
        return True

    def upload(self, channel: str):
        """
        Upload the weightfile into the condition database
        @param channel whose weightfile is uploaded
        """
        disk = channel + '.xml'
        dbase = self.config.prefix + '_' + channel
        # Up to now, we uploaded the weightfile twice, once in a subprocess and once in python
        # This makes sometimes a differences because the database cache directory can be different!
        # Hence we upload our weightfile multiple times, to make sure that it is always found.
        # subprocess.call("basf2_mva_upload --identifier '{disk}' --db_identifier '{db}'".format(disk=disk, db=dbase), shell=True)
        basf2_mva.upload(disk, dbase)
        return (disk, dbase)

    def do_all_trainings(self):
        """
        Do all trainings for which we find training data
        """
        job_list = []
        for particle in self.particles:
            for channel in particle.channels:
                filename = '{}.root'.format(channel.label)
                # weightfile = self.config.prefix + '_' + channel.label
                weightfile = channel.label + '.xml'
                if not basf2_mva.available(weightfile) and os.path.isfile(filename):
                    f = ROOT.TFile(filename)
                    if not f:
                        B2WARNING("Training of MVC failed. Couldn't find ROOT file. Ignoring channel.")
                        self.create_fake_weightfile(channel.label)
                        self.upload(channel.label)
                        continue
                    l = [m for m in f.GetListOfKeys()]
                    if not l:
                        B2WARNING("Training of MVC failed. ROOT file does not contain a tree. Ignoring channel.")
                        self.create_fake_weightfile(channel.label)
                        self.upload(channel.label)
                        continue
                    tree = l[0].ReadObj()
                    nSig = tree.GetEntries(channel.mvaConfig.target + ' == 1.0')
                    nBg = tree.GetEntries(channel.mvaConfig.target + ' != 1.0')
                    if nSig < Teacher.MinimumNumberOfMVASamples:
                        B2WARNING("Training of MVC failed."
                                  "Tree contains too few signal events {}. Ignoring channel.".format(nSig))
                        self.create_fake_weightfile(channel.label)
                        self.upload(channel.label)
                        continue
                    if nBg < Teacher.MinimumNumberOfMVASamples:
                        B2WARNING("Training of MVC failed."
                                  "Tree contains too few bckgrd events {}. Ignoring channel.".format(nBg))
                        self.create_fake_weightfile(channel.label)
                        self.upload(channel.label)
                        continue

                    command = (
                        "{externTeacher} --method '{method}' --target_variable '{target}' "
                        "--treename variables --datafile '{prefix}.root' "
                        "--signal_class 1 --variables '{variables}' --identifier '{prefix}.xml' {config} "
                        "> '{prefix}'.log 2>&1".format(
                            externTeacher=self.config.externTeacher,
                            method=channel.mvaConfig.method,
                            config=channel.mvaConfig.config,
                            target=channel.mvaConfig.target,
                            variables="' '".join(channel.mvaConfig.variables),
                            prefix=channel.label))
                    B2INFO("Used following command to invoke teacher\n" + command)
                    job_list.append((channel.label, command))

        p = multiprocessing.Pool(None, maxtasksperchild=1)
        func = functools.partial(subprocess.call, shell=True)
        p.map(func, [c for _, c in job_list])
        p.close()
        p.join()

        weightfiles = []
        for name, _ in job_list:
            if not basf2_mva.available(name + '.xml'):
                B2WARNING("Training of MVC failed. For unknown reasons, check the logfile")
                self.create_fake_weightfile(name)
            weightfiles.append(self.upload(name))
        return weightfiles


def convert_legacy_training(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration):
    """
    Convert an old FEI training into the new format.
    The old format used hashes for the weightfiles, the hashes can be converted to the new naming scheme
    using the Summary.pickle file outputted by the FEIv3. This file must be passes by the parameter configuration.legacy.
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    """
    summary = pickle.load(open(configuration.legacy, 'rb'))
    channel2lists = {k: v[2] for k, v in summary['channel2lists'].items()}

    teacher = Teacher(particles, configuration)

    for particle in particles:
        for channel in particle.channels:
            new_weightfile = configuration.prefix + '_' + channel.label
            old_weightfile = configuration.prefix + '_' + channel2lists[channel.label.replace('Jpsi', 'J/psi')]
            if not basf2_mva.available(new_weightfile):
                if old_weightfile is None or not basf2_mva.available(old_weightfile):
                    Teacher.create_fake_weightfile(channel.label)
                    teacher.upload(channel.label)
                else:
                    basf2_mva.download(old_weightfile, channel.label + '.xml')
                    teacher.upload(channel.label)


def get_stages_from_particles(particles: typing.Sequence[config.Particle]):
    """
    Returns the hierarchical structure of the FEI.
    Each stage depends on the particles in the previous stage.
    The final stage is empty (meaning everything is done, and the training is finished at this point).
    @param particles list of config.Particle objects
    """
    stages = [
        [p for p in particles if p.name in ['e+', 'K+', 'pi+', 'mu+', 'gamma', 'p+', 'K_L0', 'Lambda0']],
        [p for p in particles if p.name in ['pi0', 'J/psi']],
        [p for p in particles if p.name in ['K_S0']],
        [p for p in particles if p.name in ['D+', 'D0', 'D_s+']],
        [p for p in particles if p.name in ['D*+', 'D*0', 'D_s*+']],
        [p for p in particles if p.name in ['B0', 'B+', 'B_s+']],
        []
    ]

    for p in particles:
        if p.name not in [p.name for stage in stages for p in stage]:
            raise RuntimeError("Unknown particle {}: Not implemented in FEI".format(p.name))

    return stages


def do_trainings(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration):
    """
    Performs the training of mva classifiers for all available training data,
    this function must be either called by the user after each stage of the FEI during training,
    or (more likely) is called by the distributed.py script after  merging the outputs of all jobs,
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    @return list of tuple with weightfile on disk and identifier in database for all trained classifiers
    """
    teacher = Teacher(particles, configuration)
    return teacher.do_all_trainings()


def save_summary(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration, cache: int):
    """
    Creates the Summary.pickle, which is used to keep track of the stage during the training,
    and can be used later to investigate which configuration was used exactly to create the training.
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    @param cache current cache level
    """
    configuration = config.FeiConfiguration(configuration.prefix, cache, configuration.b2bii,
                                            configuration.monitor, configuration.legacy, configuration.externTeacher,
                                            configuration.training)
    # Backup existing Summary.pickle files
    for i in [8, 7, 6, 5, 4, 3, 2, 1, 0]:
        if os.path.isfile('Summary.pickle.backup_{}'.format(i)):
            shutil.copyfile('Summary.pickle.backup_{}'.format(i), 'Summary.pickle.backup_{}'.format(i+1))
    if os.path.isfile('Summary.pickle'):
        shutil.copyfile('Summary.pickle', 'Summary.pickle.backup_0')
    pickle.dump((particles, configuration), open('Summary.pickle', 'wb'))


def get_path(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration) -> FeiState:
    """
    The most important function of the FEI.
    This creates the FEI path for training/fitting (both terms are equal), and application/inference (both terms are equal).
    The whole FEI is defined by the particles which are reconstructed (see default_channels.py)
    and the configuration (see config.py).

    TRAINING
    For training this function is called multiple times, each time the FEI reconstructs one more stage in the hierarchical structure
    i.e. we start with FSP, pi0, KS_0, D, D*, and with B mesons. You have to set configuration.training to True for training mode.
    All weightfiles created during the training will be strored in your local database.
    If you want to use the FEI training everywhere without copying this database by hand, you have to upload your local database
    to the central database first (see documentation for the Belle2 Condition Database).

    APPLICATION
    For application you call this function once, and it returns the whole path which will reconstruct B mesons
    with an associated signal probability. You have to set configuration.training to False for application mode.

    MONITORING
    You can always turn on the monitoring (configuration.monitor = True),
    to write out ROOT Histograms of many quantities for each stage,
    using these histograms you can use the printReporting.py or latexReporting.py scripts to automatically create pdf files.

    LEGACY
    This function can also use old FEI trainings (version 3), just pass the Summary.pickle file of the old training,
    and the weightfiles will be automatically converted to the new nameing scheme.

    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    """
    print("""
    ____ _  _ _    _       ____ _  _ ____ _  _ ___    _ _  _ ___ ____ ____ ___  ____ ____ ___ ____ ___ _ ____ _  _
    |___ |  | |    |       |___ |  | |___ |\ |  |     | |\ |  |  |___ |__/ |__] |__/ |___  |  |__|  |  | |  | |\ |
    |    |__| |___ |___    |___  \/  |___ | \|  |     | | \|  |  |___ |  \ |    |  \ |___  |  |  |  |  | |__| | \|

    Author: Thomas Keck 2014 - 2017
    Please cite my PhD thesis
    """)

    # The cache parameter of the configuration object is used during training to keep track,
    # which reconstruction steps are already performed.
    # For fitting/training we start by default with -1, meaning we still have to create the TrainingDataInformation,
    # which is used to determine the number of candidates we have to write out for the FSP trainings in stage 0.
    # For inference/application we start by default with 0, because we don't need the TrainingDataInformation in stage 0.
    # During the training we save the particles and configuration (including the current cache stage) in the Summary.pickle object.
    if configuration.cache is None:
        if os.path.isfile('Summary.pickle'):
            print("Cache: Replaced particles and configuration with the ones from Summary.pickle!")
            particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
            cache = configuration.cache
        else:
            if configuration.training:
                cache = -1
            else:
                cache = 0
    else:
        cache = configuration.cache

    # Now we start building the training or application path
    path = basf2.create_path()

    # There are in total 7 stages.
    # For training we start with -1 and go to 7 one stage at a time
    # For application we can run tage 0 to 7 at once
    stages = get_stages_from_particles(particles)

    # If the user provided a Summary.pickle file of a FEIv3 training we
    # convert the old weightfiles (with hashes), to the new naming scheme.
    # Afterwards the algorithm runs as usual
    if configuration.legacy is not None:
        convert_legacy_training(particles, configuration)

    # During the training we require the number of MC particles in the whole processed
    # data sample, because we don't want to write out billions of e.g. pion candidates.
    # Knowing the total amount of MC particles we can write out only every e.g. 10th candidate
    # That's why we have to write out the TrainingDataInformation before doing anything during the training phase.
    # During application we only need this if we run in monitor mode, and want to write out a summary in the end,
    # the summary contains efficiency, and the efficiency calculation requires the total number of MC particles.
    training_data_information = TrainingDataInformation(particles)
    if cache < 0 and configuration.training:
        print("Stage 0: Run over all files to count the number of events and McParticles")
        path.add_path(training_data_information.reconstruct())
        if configuration.training:
            save_summary(particles, configuration, 0)
        return FeiState(path, 0, [])
    elif not configuration.training and configuration.monitor:
        path.add_path(training_data_information.reconstruct())

    # We load the Final State particles
    # It is assumed that the user takes care of adding RootInput, Geometry, and everything
    # which is required to read in data, so we directly start to load the FSP particles
    # used by the FEI.
    loader = FSPLoader(particles, configuration)
    if cache < 1:
        print("Stage 0: Load FSP particles")
        path.add_path(loader.reconstruct())

    # Now we reconstruct each stage one after another.
    # Each stage consists of two parts:
    # PreReconstruction (before the mva method was applied):
    #   - Particle combination
    #   - Do vertex fitting
    #   - Some simple cuts and best candidate selection
    # PostReconstruction (after the mva method was applied):
    #   - Apply the mva method
    #   - Apply cuts on the mva output and best candidate selection
    #
    # If the weightfiles for the PostReconstruction are not available for the current stage and we are in training mode,
    # we have to create the training data. The training itself is done by the do_trainings function which is called
    # either by the user after each step, or by the distributed.py script
    #
    # If the weightfiles for the PostReconstruction are not available for the current stage and we are not in training mode,
    # we keep going, as soon as the user will call process on the produced path he will get an error message that the
    # weightfiles are missing.
    #
    # Finally we keep track of the ParticleLists we use, so the user ca run the RemoveParticles module to reduce the size of the
    # intermediate output of RootOutput.
    used_lists = []
    for stage, stage_particles in enumerate(stages):
        pre_reconstruction = PreReconstruction(stage_particles, configuration)
        if cache <= stage:
            print("Stage {}: PreReconstruct particles: ".format(stage), [p.name for p in stage_particles])
            path.add_path(pre_reconstruction.reconstruct())

        post_reconstruction = PostReconstruction(stage_particles, configuration)
        if configuration.training and not post_reconstruction.available():
            print("Stage {}: Create training data for particles: ".format(stage), [p.name for p in stage_particles])
            mc_counts = training_data_information.get_mc_counts()
            training_data = TrainingData(stage_particles, configuration, mc_counts)
            path.add_path(training_data.reconstruct())
            used_lists += [channel.name for particle in stage_particles for channel in particle.channels]
            break
        if cache <= stage+1:
            path.add_path(post_reconstruction.reconstruct())
        used_lists += [particle.identifier for particle in stage_particles]

    # If we run in monitor mode we are insterested in the ModuleStatistics,
    # these statistics contain the runtime for each module which was run.
    if configuration.monitor:
        output = register_module('RootOutput')
        output.param('outputFileName', 'Monitor_ModuleStatistics.root')
        output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size
        output.param('branchNamesPersistent', ['ProcessStatistics'])
        output.param('ignoreCommandLineOverride', True)
        path.add_module(output)

    # As metioned above the FEI keeps track of the stages which are already reconstructed during the training
    # so we write out the Summary.pickle here, and increase the stage by one.
    if configuration.training or configuration.monitor:
        save_summary(particles, configuration, stage+1)

    # Finally we return the path, the stage and the used lists to the user.
    return FeiState(path, stage+1, plists=used_lists)
