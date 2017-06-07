#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2017

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
    Contains the relevant information about the used training data
    """
    def __init__(self, particles: typing.Sequence[config.Particle]):
        """
        Create a new TrainingData object
        @param particles list of config.Particle objects
        """
        self.particles = particles
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
    Steers the loading of FSP particles
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration):
        """
        Create a new FSPLoader object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        """
        self.particles = particles
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
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration,
                 mc_counts: typing.Mapping[int, typing.Mapping[str, float]]):
        """
        Create a new TrainingData object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        @param mc_counts containing number of MC Particles
        """
        self.particles = particles
        self.config = config
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
    Steers the reconstruction before the mva method of all particles in one stage.
    Includes:
        - Particle combination
        - MC Matching
        - Vertex Fitting
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration):
        """
        Create a new PreReconstruction object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        """
        self.particles = particles
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
    Steers the reconstruction using the mva method and everything after it of all particles in one stage.
    Includes:
        - MVA
        - Copy all particle lists in a common one
        - Tag unique signal
    """
    def __init__(self, particles: typing.Sequence[config.Particle], config: config.FeiConfiguration):
        """
        Create a new PostReconstruction object
        @param particles list of config.Particle objects
        @param config config.FeiConfiguration object
        """
        self.particles = particles
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
                variablesToNTuple(particle.identifier, variables, treename='variables',
                                  filename=config.removeJPsiSlash(filename), path=path)

        return path


class Teacher(object):
    """
    Performs all necessary trainings for all training data files which are available but
    without a weightfile.
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
        self.particles = particles
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

        for name, _ in job_list:
            if not basf2_mva.available(name + '.xml'):
                B2WARNING("Training of MVC failed. For unknown reasons, check the logfile")
                self.create_fake_weightfile(name)
            self.upload(name)


def convert_legacy_training(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration):
    """
    Convert an old FEI training into the new format
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    @param old_prefix old database prefix
    @param summary_file contains the configuration of the revious FEI algorithm
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
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    """
    # TODO Read particles and configuration from Summary.pickle!
    # So user does not have to provide two steering files
    teacher = Teacher(particles, configuration)
    teacher.do_all_trainings()


def get_path(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration) -> FeiState:
    """
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

    if configuration.cache is None:
        if os.path.isfile('Summary.pickle'):
            print("Cache: Replaced particles and configuration with the ones from Summary.pickle!")
            particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
            configuration = config.FeiConfiguration(configuration.prefix, configuration.cache + 1, configuration.b2bii,
                                                    configuration.monitor, configuration.legacy, configuration.externTeacher,
                                                    configuration.training)
            cache = configuration.cache
        else:
            cache = 0
    else:
        cache = configuration.cache

    path = basf2.create_path()
    stages = get_stages_from_particles(particles)

    if configuration.legacy is not None:
        convert_legacy_training(particles, configuration)

    training_data_information = TrainingDataInformation(particles)
    if cache < 0 and configuration.training:
        print("Stage 0: Run over all files to count the number of events and McParticles")
        path.add_path(training_data_information.reconstruct())
        return FeiState(path, 0, [])
    elif not configuration.training and configuration.monitor:
        path.add_path(training_data_information.reconstruct())

    loader = FSPLoader(particles, configuration)
    if cache < 1:
        print("Stage 0: Load FSP particles")
        path.add_path(loader.reconstruct())

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

    if configuration.monitor:
        output = register_module('RootOutput')
        output.param('outputFileName', 'Monitor_ModuleStatistics.root')
        output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size
        output.param('branchNamesPersistent', ['ProcessStatistics'])
        output.param('ignoreCommandLineOverride', True)
        path.add_module(output)

    if configuration.training:
        pickle.dump((particles, configuration), open('Summary.pickle', 'wb'))

    return FeiState(path, stage+1, plists=used_lists)
