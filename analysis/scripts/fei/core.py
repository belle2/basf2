#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond SUPPRESS_DOXYGEN

"""
 The Full Event Interpretation Algorithm

 Some basic facts:
  - The algorithm will automatically reconstruct B mesons and calculate a signal probability for each candidate.
  - It can be used for hadronic and semileptonic tagging.
  - The algorithm has to be trained on MC, and can afterwards be applied on data.
  - The training requires O(100) million MC events
  - The weight files are stored in the Belle II Condition database

 Read this file if you want to understand the technical details of the FEI.

 The FEI follows a hierarchical approach.
 There are 7 stages:
   (Stage -1: Write out information about the provided data sample)
   Stage 0: Final State Particles (FSP)
   Stage 1: pi0, J/Psi, Lambda0
   Stage 2: K_S0, Sigma+
   Stage 3: D and Lambda_c mesons
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
 to train all the necessary multivariate classifiers.
"""

# Import basf2
import basf2
from basf2 import B2INFO, B2WARNING, B2ERROR
import pybasf2
import modularAnalysis as ma
import b2bii

# Should come after basf2 import
import pdg
from fei import config
import basf2_mva

# Standard python modules
import collections
import os
import shutil
import typing
import pickle
import re
import functools
import subprocess
import multiprocessing

# Simple object containing the output of fei
FeiState = collections.namedtuple('FeiState', 'path, stage, plists, fsplists, excludelists')


class TrainingDataInformation:
    """
    Contains the relevant information about the used training data.
    Basically we write out the number of MC particles in the whole dataset.
    This numbers we can use to calculate what fraction of candidates we have to write
    out as TrainingData to get a reasonable amount of candidates to train on
    (too few candidates will lead to heavy overtraining, too many won't fit into memory).
    Secondly we can use this information for the generation of the monitoring pdfs,
    where we calculate reconstruction efficiencies.
    """

    def __init__(self, particles: typing.Sequence[config.Particle], outputPath: str = ''):
        """
        Create a new TrainingData object
        @param particles list of config.Particle objects
        @param outputPath path to the output directory
        """
        #: list of config.Particle objects
        self.particles = particles
        #: filename
        self.filename = os.path.join(outputPath, 'mcParticlesCount.root')

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
        pdgs = {abs(pdg.from_name(particle.name)) for particle in self.particles}

        path = basf2.create_path()
        module = basf2.register_module('VariablesToHistogram')
        module.set_name("VariablesToHistogram_MCCount")
        module.param('variables', [(f'NumberOfMCParticlesInEvent({pdg})', 100, -0.5, 99.5) for pdg in pdgs])
        module.param('fileName', self.filename)
        module.param('ignoreCommandLineOverride', True)
        path.add_module(module)
        return path

    def get_mc_counts(self):
        """
        Read out the number of MC particles from the file created by reconstruct
        """
        # Unique absolute pdg-codes of all particles
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        root_file = ROOT.TFile.Open(self.filename, 'read')
        mc_counts = {}

        for key in root_file.GetListOfKeys():
            variable = ROOT.Belle2.MakeROOTCompatible.invertMakeROOTCompatible(key.GetName())
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
        mc_counts[0]['sum'] = hist.GetEntries()  # this is the total number of ALL events, does not matter which hist we take
        root_file.Close()
        return mc_counts


class FSPLoader:
    """
    Steers the loading of FSP particles.
    This does NOT include RootInput, Geometry or anything required before loading FSPs,
    the user has to add this himself (because it depends on the MC campaign and if you want
    to use Belle or Belle II).
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

    def get_fsp_lists(self) -> typing.List[str]:
        """
        Returns a list of FSP particle lists which are used in the FEI.
        This is used to create the RootOutput module.
        """
        fsps = ['K+:FSP', 'pi+:FSP', 'e+:FSP', 'mu+:FSP', 'p+:FSP', 'gamma:FSP', 'K_S0:V0', 'Lambda0:V0', 'K_L0:FSP', 'gamma:V0']
        if b2bii.isB2BII():
            fsps += ['pi0:FSP']
        return fsps

    def reconstruct(self) -> pybasf2.Path:
        """
        Returns pybasf2.Path which loads the FSP Particles
        """
        path = basf2.create_path()

        if b2bii.isB2BII():
            ma.fillParticleLists([('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                  ('mu+:FSP', ''), ('p+:FSP', '')], writeOut=True, path=path)
            for outputList, inputList in [('gamma:FSP', 'gamma:mdst'), ('K_S0:V0', 'K_S0:mdst'),
                                          ('Lambda0:V0', 'Lambda0:mdst'), ('K_L0:FSP', 'K_L0:mdst'),
                                          ('pi0:FSP', 'pi0:mdst'), ('gamma:V0', 'gamma:v0mdst')]:
                ma.copyParticles(outputList, inputList, writeOut=True, path=path)
        else:
            ma.fillParticleLists([('K+:FSP', ''), ('pi+:FSP', ''), ('e+:FSP', ''),
                                  ('mu+:FSP', ''), ('gamma:FSP', ''),
                                  ('p+:FSP', ''), ('K_L0:FSP', '')], writeOut=True, path=path)
            ma.fillParticleList('K_S0:V0 -> pi+ pi-', '', writeOut=True, path=path)
            ma.fillParticleList('Lambda0:V0 -> p+ pi-', '', writeOut=True, path=path)
            ma.fillConvertedPhotonsList('gamma:V0 -> e+ e-', '', writeOut=True, path=path)

        if self.config.monitor:
            names = ['e+', 'K+', 'pi+', 'mu+', 'gamma', 'K_S0', 'p+', 'K_L0', 'Lambda0', 'pi0']
            filename = os.path.join(self.config.monitoring_path, 'Monitor_FSPLoader.root')
            pdgs = {abs(pdg.from_name(name)) for name in names}
            variables = [(f'NumberOfMCParticlesInEvent({pdg})', 100, -0.5, 99.5) for pdg in pdgs]
            ma.variablesToHistogram('', variables=variables, filename=filename, ignoreCommandLineOverride=True, path=path)
        return path


class TrainingData:
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
        import ROOT  # noqa
        path = basf2.create_path()

        for particle in self.particles:
            pdgcode = abs(pdg.from_name(particle.name))
            nSignal = self.mc_counts[pdgcode]['sum']
            print(f"FEI-core: TrainingData: nSignal for {particle.name}: {nSignal}")

            # For D-Mesons we usually have a efficiency of 10^-3 including branching fraction
            if pdgcode > 400:
                nSignal /= 1000
            # For B-Mesons we usually have a efficiency of 10^-4 including branching fraction
            if pdgcode > 500:
                nSignal /= 10000

            for channel in particle.channels:
                weightfile = f'{channel.label}.xml'
                if basf2_mva.available(weightfile):
                    B2INFO(f"FEI-core: Skipping preparing Training Data for {weightfile}, already available")
                    continue
                filename = 'training_input.root'

                # nBackground = nEvents * nBestCandidates
                nBackground = self.mc_counts[0]['sum'] * channel.preCutConfig.bestCandidateCut
                inverseSamplingRates = {}
                # For some very pure channels (Jpsi), this sampling can be too aggressive and training fails.
                # It can therefore be disabled in the preCutConfig.
                if nBackground > Teacher.MaximumNumberOfMVASamples and not channel.preCutConfig.noBackgroundSampling:
                    inverseSamplingRates[0] = max(
                        1, int((int(nBackground / Teacher.MaximumNumberOfMVASamples) + 1) * channel.preCutConfig.bkgSamplingFactor))
                elif channel.preCutConfig.bkgSamplingFactor > 1:
                    inverseSamplingRates[0] = int(channel.preCutConfig.bkgSamplingFactor)

                if nSignal > Teacher.MaximumNumberOfMVASamples and not channel.preCutConfig.noSignalSampling:
                    inverseSamplingRates[1] = int(nSignal / Teacher.MaximumNumberOfMVASamples) + 1

                spectators = [channel.mvaConfig.target] + list(channel.mvaConfig.spectators.keys())
                if channel.mvaConfig.sPlotVariable is not None:
                    spectators.append(channel.mvaConfig.sPlotVariable)

                if self.config.monitor:
                    hist_variables = ['mcErrors', 'mcParticleStatus'] + channel.mvaConfig.variables + spectators
                    hist_variables_2d = [(x, channel.mvaConfig.target)
                                         for x in channel.mvaConfig.variables + spectators if x is not channel.mvaConfig.target]
                    hist_filename = os.path.join(self.config.monitoring_path, 'Monitor_TrainingData.root')
                    ma.variablesToHistogram(channel.name, variables=config.variables2binnings(hist_variables),
                                            variables_2d=config.variables2binnings_2d(hist_variables_2d),
                                            filename=hist_filename,
                                            ignoreCommandLineOverride=True,
                                            directory=config.removeJPsiSlash(f'{channel.label}'), path=path)

                teacher = basf2.register_module('VariablesToNtuple')
                teacher.set_name(f'VariablesToNtuple_{channel.name}')
                teacher.param('fileName', filename)
                teacher.param('treeName', ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible(f'{channel.label} variables'))
                teacher.param('variables', channel.mvaConfig.variables + spectators)
                teacher.param('particleList', channel.name)
                teacher.param('sampling', (channel.mvaConfig.target, inverseSamplingRates))
                teacher.param('ignoreCommandLineOverride', True)
                path.add_module(teacher)
        return path


class PreReconstruction:
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

                if (len(channel.daughters) == 1) and (pdg.from_name(
                        channel.daughters[0].split(':')[0]) == pdg.from_name(particle.name)):
                    ma.cutAndCopyList(channel.name, channel.daughters[0], channel.preCutConfig.userCut, writeOut=True, path=path)
                    v2EI = basf2.register_module('VariablesToExtraInfo')
                    v2EI.set_name(f'VariablesToExtraInfo_{channel.name}')
                    v2EI.param('particleList', channel.name)
                    v2EI.param('variables', {f'constant({channel.decayModeID})': 'decayModeID'})
                    # suppress warning that decay mode ID won't be overwritten if it already exists
                    v2EI.set_log_level(basf2.logging.log_level.ERROR)
                    path.add_module(v2EI)
                else:
                    ma.reconstructDecay(channel.decayString, channel.preCutConfig.userCut, channel.decayModeID,
                                        writeOut=True, path=path)
                if self.config.monitor:
                    ma.matchMCTruth(channel.name, path=path)
                    bc_variable = channel.preCutConfig.bestCandidateVariable
                    if self.config.monitor == 'simple':
                        hist_variables = [channel.mvaConfig.target, 'extraInfo(decayModeID)']
                        hist_variables_2d = [(channel.mvaConfig.target, 'extraInfo(decayModeID)')]
                    else:
                        hist_variables = [bc_variable, 'mcErrors', 'mcParticleStatus',
                                          channel.mvaConfig.target] + list(channel.mvaConfig.spectators.keys())
                        hist_variables_2d = [(bc_variable, channel.mvaConfig.target),
                                             (bc_variable, 'mcErrors'),
                                             (bc_variable, 'mcParticleStatus')]
                        for specVar in channel.mvaConfig.spectators:
                            hist_variables_2d.append((bc_variable, specVar))
                            hist_variables_2d.append((channel.mvaConfig.target, specVar))
                    filename = os.path.join(self.config.monitoring_path, 'Monitor_PreReconstruction_BeforeRanking.root')
                    ma.variablesToHistogram(
                        channel.name,
                        variables=config.variables2binnings(hist_variables),
                        variables_2d=config.variables2binnings_2d(hist_variables_2d),
                        filename=filename,
                        ignoreCommandLineOverride=True,
                        directory=f'{channel.label}',
                        path=path)

                if channel.preCutConfig.bestCandidateMode == 'lowest':
                    ma.rankByLowest(channel.name,
                                    channel.preCutConfig.bestCandidateVariable,
                                    channel.preCutConfig.bestCandidateCut,
                                    'preCut_rank',
                                    path=path)
                elif channel.preCutConfig.bestCandidateMode == 'highest':
                    ma.rankByHighest(channel.name,
                                     channel.preCutConfig.bestCandidateVariable,
                                     channel.preCutConfig.bestCandidateCut,
                                     'preCut_rank',
                                     path=path)
                else:
                    raise RuntimeError(f'Unknown bestCandidateMode {repr(channel.preCutConfig.bestCandidateMode)}')

                if 'gamma' in channel.decayString and channel.pi0veto:
                    ma.buildRestOfEvent(channel.name, path=path)
                    Ddaughter_roe_path = basf2.Path()
                    deadEndPath = basf2.Path()
                    ma.signalSideParticleFilter(channel.name, '', Ddaughter_roe_path, deadEndPath)
                    ma.fillParticleList('gamma:roe', 'isInRestOfEvent == 1', path=Ddaughter_roe_path)

                    matches = list(re.finditer('gamma', channel.decayString))
                    pi0lists = []
                    for igamma in range(len(matches)):
                        start, end = matches[igamma-1].span()
                        tempString = f'{channel.decayString[:start]}^gamma{channel.decayString[end:]}'
                        ma.fillSignalSideParticleList(f'gamma:sig_{igamma}', tempString, path=Ddaughter_roe_path)
                        ma.reconstructDecay(f'pi0:veto_{igamma} -> gamma:sig_{igamma} gamma:roe', '', path=Ddaughter_roe_path)
                        pi0lists.append(f'pi0:veto_{igamma}')
                    ma.copyLists('pi0:veto', pi0lists, writeOut=False, path=Ddaughter_roe_path)
                    ma.rankByLowest('pi0:veto', 'abs(dM)', 1, path=Ddaughter_roe_path)
                    ma.matchMCTruth('pi0:veto', path=Ddaughter_roe_path)
                    ma.variableToSignalSideExtraInfo(
                        'pi0:veto',
                        {
                            'InvM': 'pi0vetoMass',
                            'formula((daughter(0,E)-daughter(1,E))/(daughter(0,E)+daughter(1,E)))': 'pi0vetoEnergyAsymmetry',
                        },
                        path=Ddaughter_roe_path
                    )
                    path.for_each('RestOfEvent', 'RestOfEvents', Ddaughter_roe_path)

                if self.config.monitor:
                    filename = os.path.join(self.config.monitoring_path, 'Monitor_PreReconstruction_AfterRanking.root')
                    if self.config.monitor != 'simple':
                        hist_variables += ['extraInfo(preCut_rank)']
                        hist_variables_2d += [('extraInfo(preCut_rank)', channel.mvaConfig.target),
                                              ('extraInfo(preCut_rank)', 'mcErrors'),
                                              ('extraInfo(preCut_rank)', 'mcParticleStatus')]
                        for specVar in channel.mvaConfig.spectators:
                            hist_variables_2d.append(('extraInfo(preCut_rank)', specVar))
                    ma.variablesToHistogram(
                        channel.name,
                        variables=config.variables2binnings(hist_variables),
                        variables_2d=config.variables2binnings_2d(hist_variables_2d),
                        filename=filename,
                        ignoreCommandLineOverride=True,
                        directory=f'{channel.label}',
                        path=path)
                # If we are not in monitor mode we do the mc matching now,
                # otherwise we did it above already!
                elif self.config.training:
                    ma.matchMCTruth(channel.name, path=path)

                if b2bii.isB2BII() and particle.name in ['K_S0', 'Lambda0']:
                    pvfit = basf2.register_module('ParticleVertexFitter')
                    pvfit.set_name(f'ParticleVertexFitter_{channel.name}')
                    pvfit.param('listName', channel.name)
                    pvfit.param('confidenceLevel', channel.preCutConfig.vertexCut)
                    pvfit.param('vertexFitter', 'KFit')
                    pvfit.param('fitType', 'vertex')
                    pvfit.set_log_level(basf2.logging.log_level.ERROR)  # let's not produce gigabytes of uninteresting warnings
                    path.add_module(pvfit)
                elif re.findall(r"[\w']+", channel.decayString).count('pi0') > 1 and particle.name != 'pi0':
                    basf2.B2INFO(f"Ignoring vertex fit for {channel.name} because multiple pi0 are not supported yet.")
                elif len(channel.daughters) > 1:
                    pvfit = basf2.register_module('ParticleVertexFitter')
                    pvfit.set_name(f'ParticleVertexFitter_{channel.name}')
                    pvfit.param('listName', channel.name)
                    pvfit.param('confidenceLevel', channel.preCutConfig.vertexCut)
                    pvfit.param('vertexFitter', 'KFit')
                    if particle.name in ['pi0']:
                        pvfit.param('fitType', 'mass')
                    else:
                        pvfit.param('fitType', 'vertex')
                    pvfit.set_log_level(basf2.logging.log_level.ERROR)  # let's not produce gigabytes of uninteresting warnings
                    path.add_module(pvfit)

                if self.config.monitor:
                    if self.config.monitor == 'simple':
                        hist_variables = [channel.mvaConfig.target, 'extraInfo(decayModeID)']
                        hist_variables_2d = [(channel.mvaConfig.target, 'extraInfo(decayModeID)')]
                    else:
                        hist_variables = ['chiProb', 'mcErrors', 'mcParticleStatus',
                                          channel.mvaConfig.target] + list(channel.mvaConfig.spectators.keys())
                        hist_variables_2d = [('chiProb', channel.mvaConfig.target),
                                             ('chiProb', 'mcErrors'),
                                             ('chiProb', 'mcParticleStatus')]
                        for specVar in channel.mvaConfig.spectators:
                            hist_variables_2d.append(('chiProb', specVar))
                            hist_variables_2d.append((channel.mvaConfig.target, specVar))
                    filename = os.path.join(self.config.monitoring_path, 'Monitor_PreReconstruction_AfterVertex.root')
                    ma.variablesToHistogram(
                        channel.name,
                        variables=config.variables2binnings(hist_variables),
                        variables_2d=config.variables2binnings_2d(hist_variables_2d),
                        filename=filename,
                        ignoreCommandLineOverride=True,
                        directory=f'{channel.label}',
                        path=path)

        return path


class PostReconstruction:
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
                weightfile = f'{channel.label}.xml'
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
        import ROOT  # noqa
        path = basf2.create_path()

        for particle in self.particles:
            for channel in particle.channels:
                expert = basf2.register_module('MVAExpert')
                expert.set_name(f'MVAExpert_{channel.name}')
                if self.config.training:
                    expert.param('identifier', f'{channel.label}.xml')
                else:
                    expert.param('identifier', f'{self.config.prefix}_{channel.label}')
                expert.param('extraInfoName', 'SignalProbability')
                expert.param('listNames', [channel.name])
                # suppress warning that signal probability won't be overwritten if it already exists
                expert.set_log_level(basf2.logging.log_level.ERROR)
                path.add_module(expert)

                if self.config.monitor:
                    if self.config.monitor == 'simple':
                        hist_variables = [channel.mvaConfig.target, 'extraInfo(decayModeID)']
                        hist_variables_2d = [(channel.mvaConfig.target, 'extraInfo(decayModeID)')]
                    else:
                        hist_variables = ['mcErrors',
                                          'mcParticleStatus',
                                          'extraInfo(SignalProbability)',
                                          channel.mvaConfig.target,
                                          'extraInfo(decayModeID)'] + list(channel.mvaConfig.spectators.keys())
                        hist_variables_2d = [('extraInfo(SignalProbability)', channel.mvaConfig.target),
                                             ('extraInfo(SignalProbability)', 'mcErrors'),
                                             ('extraInfo(SignalProbability)', 'mcParticleStatus'),
                                             ('extraInfo(decayModeID)', channel.mvaConfig.target),
                                             ('extraInfo(decayModeID)', 'mcErrors'),
                                             ('extraInfo(decayModeID)', 'mcParticleStatus')]
                        for specVar in channel.mvaConfig.spectators:
                            hist_variables_2d.append(('extraInfo(SignalProbability)', specVar))
                            hist_variables_2d.append(('extraInfo(decayModeID)', specVar))
                            hist_variables_2d.append((channel.mvaConfig.target, specVar))
                    filename = os.path.join(self.config.monitoring_path, 'Monitor_PostReconstruction_AfterMVA.root')
                    ma.variablesToHistogram(
                        channel.name,
                        variables=config.variables2binnings(hist_variables),
                        variables_2d=config.variables2binnings_2d(hist_variables_2d),
                        filename=filename,
                        ignoreCommandLineOverride=True,
                        directory=f'{channel.label}',
                        path=path)

            cutstring = ''
            if particle.postCutConfig.value > 0.0:
                cutstring = f'{particle.postCutConfig.value} < extraInfo(SignalProbability)'

            ma.mergeListsWithBestDuplicate(particle.identifier, [c.name for c in particle.channels],
                                           variable='particleSource', writeOut=True, path=path)

            if self.config.monitor:
                if self.config.monitor == 'simple':
                    hist_variables = [particle.mvaConfig.target, 'extraInfo(decayModeID)']
                    hist_variables_2d = [(particle.mvaConfig.target, 'extraInfo(decayModeID)')]
                else:
                    hist_variables = ['mcErrors',
                                      'mcParticleStatus',
                                      'extraInfo(SignalProbability)',
                                      particle.mvaConfig.target,
                                      'extraInfo(decayModeID)'] + list(particle.mvaConfig.spectators.keys())
                    hist_variables_2d = [('extraInfo(decayModeID)', particle.mvaConfig.target),
                                         ('extraInfo(decayModeID)', 'mcErrors'),
                                         ('extraInfo(decayModeID)', 'mcParticleStatus')]
                    for specVar in particle.mvaConfig.spectators:
                        hist_variables_2d.append(('extraInfo(SignalProbability)', specVar))
                        hist_variables_2d.append(('extraInfo(decayModeID)', specVar))
                        hist_variables_2d.append((particle.mvaConfig.target, specVar))
                filename = os.path.join(self.config.monitoring_path, 'Monitor_PostReconstruction_BeforePostCut.root')
                ma.variablesToHistogram(
                    particle.identifier,
                    variables=config.variables2binnings(hist_variables),
                    variables_2d=config.variables2binnings_2d(hist_variables_2d),
                    filename=filename,
                    ignoreCommandLineOverride=True,
                    directory=config.removeJPsiSlash(f'{particle.identifier}'),
                    path=path)

            ma.applyCuts(particle.identifier, cutstring, path=path)

            if self.config.monitor:
                filename = os.path.join(self.config.monitoring_path, 'Monitor_PostReconstruction_BeforeRanking.root')
                ma.variablesToHistogram(
                    particle.identifier,
                    variables=config.variables2binnings(hist_variables),
                    variables_2d=config.variables2binnings_2d(hist_variables_2d),
                    filename=filename,
                    ignoreCommandLineOverride=True,
                    directory=config.removeJPsiSlash(f'{particle.identifier}'),
                    path=path)

            ma.rankByHighest(particle.identifier, 'extraInfo(SignalProbability)',
                             particle.postCutConfig.bestCandidateCut, 'postCut_rank', path=path)

            uniqueSignal = basf2.register_module('TagUniqueSignal')
            uniqueSignal.param('particleList', particle.identifier)
            uniqueSignal.param('target', particle.mvaConfig.target)
            uniqueSignal.param('extraInfoName', 'uniqueSignal')
            uniqueSignal.set_name(f'TagUniqueSignal_{particle.identifier}')
            # suppress warning that unique signal extra info won't be overwritten if it already exists
            uniqueSignal.set_log_level(basf2.logging.log_level.ERROR)
            path.add_module(uniqueSignal)

            if self.config.monitor:
                if self.config.monitor != 'simple':
                    hist_variables += ['extraInfo(postCut_rank)']
                    hist_variables_2d += [('extraInfo(decayModeID)', 'extraInfo(postCut_rank)'),
                                          (particle.mvaConfig.target, 'extraInfo(postCut_rank)'),
                                          ('mcErrors', 'extraInfo(postCut_rank)'),
                                          ('mcParticleStatus', 'extraInfo(postCut_rank)')]
                    for specVar in particle.mvaConfig.spectators:
                        hist_variables_2d.append(('extraInfo(postCut_rank)', specVar))
                filename = os.path.join(self.config.monitoring_path, 'Monitor_PostReconstruction_AfterRanking.root')
                ma.variablesToHistogram(
                    particle.identifier,
                    variables=config.variables2binnings(hist_variables),
                    variables_2d=config.variables2binnings_2d(hist_variables_2d),
                    filename=filename,
                    ignoreCommandLineOverride=True,
                    directory=config.removeJPsiSlash(f'{particle.identifier}'),
                    path=path)

                filename = os.path.join(self.config.monitoring_path, 'Monitor_Final.root')
                if self.config.monitor == 'simple':
                    hist_variables = ['extraInfo(uniqueSignal)', 'extraInfo(decayModeID)']
                    hist_variables_2d = [('extraInfo(uniqueSignal)', 'extraInfo(decayModeID)')]
                    ma.variablesToHistogram(
                        particle.identifier,
                        variables=config.variables2binnings(hist_variables),
                        variables_2d=config.variables2binnings_2d(hist_variables_2d),
                        filename=filename,
                        ignoreCommandLineOverride=True,
                        directory=config.removeJPsiSlash(f'{particle.identifier}'),
                        path=path)
                else:
                    variables = ['extraInfo(SignalProbability)', 'mcErrors', 'mcParticleStatus', particle.mvaConfig.target,
                                 'extraInfo(uniqueSignal)', 'extraInfo(decayModeID)'] + list(particle.mvaConfig.spectators.keys())

                    ma.variablesToNtuple(
                        particle.identifier,
                        variables,
                        treename=ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible(
                            config.removeJPsiSlash(f'{particle.identifier} variables')),
                        filename=filename,
                        ignoreCommandLineOverride=True,
                        path=path)
        return path


class Teacher:
    """
    Performs all necessary trainings for all training data files which are
    available but where there is no weight file available yet.
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
        Create a fake weight file using the trivial method, it will always return 0.0
        @param channel for which we create a fake weight file
        """
        content = f"""
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
            """
        with open(f'{channel}.xml', "w") as f:
            f.write(content)

    @staticmethod
    def check_if_weightfile_is_fake(filename: str):
        """
        Checks if the provided filename is a fake-weight file or not
        @param filename the filename of the weight file
        """
        try:
            return '<method>Trivial</method>' in open(filename).readlines()[2]
        except BaseException:
            return True
        return True

    def upload(self, channel: str):
        """
        Upload the weight file into the condition database
        @param channel whose weight file is uploaded
        """
        disk = f'{channel}.xml'
        dbase = f'{self.config.prefix}_{channel}'
        basf2_mva.upload(disk, dbase)
        print(f"FEI-core: Uploading {dbase} to localdb")
        return (disk, dbase)

    def do_all_trainings(self):
        """
        Do all trainings for which we find training data
        """
        # Always avoid the top-level 'import ROOT'.
        import ROOT  # noqa
        # FEI uses multi-threading for parallel execution of tasks therefore
        # the ROOT gui-thread is disabled, which otherwise interferes sometimes
        ROOT.PyConfig.StartGuiThread = False
        job_list = []

        all_stage_particles = get_stages_from_particles(self.particles)
        if self.config.cache is None:
            stagesToTrain = range(1, len(all_stage_particles)+1)
        else:
            stagesToTrain = [self.config.cache]

        filename = 'training_input.root'
        if os.path.isfile(filename):
            f = ROOT.TFile.Open(filename, 'read')
            if f.IsZombie():
                B2WARNING(f'Training of MVC failed: {filename}. ROOT file corrupt. No weight files will be provided.')
            elif len([k.GetName() for k in f.GetListOfKeys()]) == 0:
                B2WARNING(
                    f'Training of MVC failed: {filename}. ROOT file has no trees. No weight files will be provided.')
            else:
                for istage in stagesToTrain:
                    for particle in all_stage_particles[istage-1]:
                        for channel in particle.channels:
                            weightfile = f'{channel.label}.xml'
                            if basf2_mva.available(weightfile):
                                B2INFO(f"FEI-core: Skipping {weightfile}, already available")
                                continue
                            else:
                                treeName = ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible(f'{channel.label} variables')
                                keys = [m for m in f.GetListOfKeys() if treeName in m.GetName()]
                                if not keys:
                                    B2WARNING("Training of MVC failed. "
                                              f"Couldn't find tree for channel {channel}. Ignoring channel.")
                                    continue
                                elif len(keys) > 1:
                                    B2WARNING(f"Found more than one tree for channel {channel}. Taking first tree from: {keys}")
                                tree = keys[0].ReadObj()
                                total_entries = tree.GetEntries()
                                nSig = tree.GetEntries(f'{channel.mvaConfig.target}==1.0')
                                nBg = tree.GetEntries(f'{channel.mvaConfig.target}==0.0')
                                B2INFO(
                                    f'FEI-core: Number of events for channel: {channel.label}, '
                                    f'Total: {total_entries}, Signal: {nSig}, Background: {nBg}')
                                if nSig < Teacher.MinimumNumberOfMVASamples:
                                    B2WARNING("Training of MVC failed. "
                                              f"Tree contains too few signal events {nSig}. Ignoring channel {channel}.")
                                    self.create_fake_weightfile(channel.label)
                                    self.upload(channel.label)
                                    continue
                                if nBg < Teacher.MinimumNumberOfMVASamples:
                                    B2WARNING("Training of MVC failed. "
                                              f"Tree contains too few bckgrd events {nBg}. Ignoring channel {channel}.")
                                    self.create_fake_weightfile(channel.label)
                                    self.upload(channel.label)
                                    continue
                                variable_str = "' '".join(channel.mvaConfig.variables)

                                spectators = list(channel.mvaConfig.spectators.keys())
                                if channel.mvaConfig.sPlotVariable is not None:
                                    spectators.append(channel.mvaConfig.sPlotVariable)
                                spectators_str = "' '".join(spectators)

                                treeName = ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible(f'{channel.label} variables')
                                command = (f"{self.config.externTeacher}"
                                           f" --method '{channel.mvaConfig.method}'"
                                           f" --target_variable '{channel.mvaConfig.target}'"
                                           f" --treename '{treeName}'"
                                           f" --datafile 'training_input.root'"
                                           f" --signal_class 1"
                                           f" --variables '{variable_str}'"
                                           f" --identifier '{weightfile}'")
                                if len(spectators) > 0:
                                    command += f" --spectators '{spectators_str}'"
                                command += f" {channel.mvaConfig.config} > '{channel.label}'.log 2>&1"
                                B2INFO(f"Used following command to invoke teacher: \n {command}")
                                job_list.append((channel.label, command))
            f.Close()

        if len(job_list) > 0:
            p = multiprocessing.Pool(None, maxtasksperchild=1)
            func = functools.partial(subprocess.call, shell=True)
            p.map(func, [c for _, c in job_list])
            p.close()
            p.join()
        weightfiles = []
        for name, _ in job_list:
            if not basf2_mva.available(f'{name}.xml'):
                B2WARNING("Training of MVC failed. For unknown reasons, check the logfile", f'{name}.log')
                self.create_fake_weightfile(name)
            weightfiles.append(self.upload(name))
        return weightfiles


def convert_legacy_training(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration):
    """
    Convert an old FEI training into the new format.
    The old format used hashes for the weight files, the hashes can be converted to the new naming scheme
    using the Summary.pickle file outputted by the FEIv3. This file must be passes by the parameter configuration.legacy.
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    """
    summary = pickle.load(open(configuration.legacy, 'rb'))
    channel2lists = {k: v[2] for k, v in summary['channel2lists'].items()}

    teacher = Teacher(particles, configuration)

    for particle in particles:
        for channel in particle.channels:
            new_weightfile = f'{configuration.prefix}_{channel.label}'
            old_weightfile = f'{configuration.prefix}_{channel2lists[channel.label.replace("Jpsi", "J/psi")]}'
            if not basf2_mva.available(new_weightfile):
                if old_weightfile is None or not basf2_mva.available(old_weightfile):
                    Teacher.create_fake_weightfile(channel.label)
                    teacher.upload(channel.label)
                else:
                    basf2_mva.download(old_weightfile, f'{channel.label}.xml')
                    teacher.upload(channel.label)


def get_stages_from_particles(particles: typing.Sequence[typing.Union[config.Particle, str]]):
    """
    Returns the hierarchical structure of the FEI.
    Each stage depends on the particles in the previous stage.
    The final stage is empty (meaning everything is done, and the training is finished at this point).
    @param particles list of config.Particle or string objects
    """
    def get_pname(p):
        return p.split(":")[0] if isinstance(p, str) else p.name

    def get_plabel(p):
        return (p.split(":")[1] if isinstance(p, str) else p.label).lower()

    stages = [
        [p for p in particles if get_pname(p) in ['e+', 'K+', 'pi+', 'mu+', 'gamma', 'p+', 'K_L0']],
        [p for p in particles if get_pname(p) in ['pi0', 'J/psi', 'Lambda0']],
        [p for p in particles if get_pname(p) in ['K_S0', 'Sigma+']],
        [p for p in particles if get_pname(p) in ['D+', 'D0', 'D_s+', 'Lambda_c+'] and 'tag' not in get_plabel(p)],
        [p for p in particles if get_pname(p) in ['D*+', 'D*0', 'D_s*+'] and 'tag' not in get_plabel(p)],
        [p for p in particles if get_pname(p) in ['B0', 'B+', 'B_s0'] and 'tag' not in get_plabel(p)],
        []
    ]

    for p in particles:
        pname = get_pname(p)
        if pname not in [pname for stage in stages for p in stage]:
            raise RuntimeError(f"Unknown particle {pname}: Not implemented in FEI")

    return stages


def do_trainings(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration):
    """
    Performs the training of mva classifiers for all available training data,
    this function must be either called by the user after each stage of the FEI during training,
    or (more likely) is called by the distributed.py script after  merging the outputs of all jobs,
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    @return list of tuple with weight file on disk and identifier in database for all trained classifiers
    """
    teacher = Teacher(particles, configuration)
    return teacher.do_all_trainings()


def save_summary(particles: typing.Sequence[config.Particle],
                 configuration: config.FeiConfiguration,
                 cache: int,
                 roundMode: int = None,
                 pickleName: str = 'Summary.pickle'):
    """
    Creates the Summary.pickle, which is used to keep track of the stage during the training,
    and can be used later to investigate which configuration was used exactly to create the training.
    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    @param cache current cache level
    @param roundMode mode of current round of training
    @param pickleName name of the pickle file
    """
    if roundMode is None:
        roundMode = configuration.roundMode
    configuration = configuration._replace(cache=cache, roundMode=roundMode)
    # Backup existing Summary.pickle files
    for i in range(8, -1, -1):
        if os.path.isfile(f'{pickleName}.backup_{i}'):
            shutil.copyfile(f'{pickleName}.backup_{i}', f'{pickleName}.backup_{i+1}')
    if os.path.isfile(pickleName):
        shutil.copyfile(pickleName, f'{pickleName}.backup_0')
    pickle.dump((particles, configuration), open(pickleName, 'wb'))


def get_path(particles: typing.Sequence[config.Particle], configuration: config.FeiConfiguration) -> FeiState:
    """
    The most important function of the FEI.
    This creates the FEI path for training/fitting (both terms are equal), and application/inference (both terms are equal).
    The whole FEI is defined by the particles which are reconstructed (see default_channels.py)
    and the configuration (see config.py).

    TRAINING
    For training this function is called multiple times, each time the FEI reconstructs one more stage in the hierarchical structure
    i.e. we start with FSP, pi0, KS_0, D, D*, and with B mesons. You have to set configuration.training to True for training mode.
    All weight files created during the training will be stored in your local database.
    If you want to use the FEI training everywhere without copying this database by hand, you have to upload your local database
    to the central database first (see documentation for the Belle2 Condition Database).

    APPLICATION
    For application you call this function once, and it returns the whole path which will reconstruct B mesons
    with an associated signal probability. You have to set configuration.training to False for application mode.

    MONITORING
    You can always turn on the monitoring (configuration.monitor != False),
    to write out ROOT Histograms of many quantities for each stage,
    using these histograms you can use the printReporting.py or latexReporting.py scripts to automatically create pdf files.

    LEGACY
    This function can also use old FEI trainings (version 3), just pass the Summary.pickle file of the old training,
    and the weight files will be automatically converted to the new naming scheme.

    @param particles list of config.Particle objects
    @param config config.FeiConfiguration object
    """
    print(r"""
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
    # RoundMode plays a similar role as cache,
    # it is used to keep track in which phase within a stage the basf2 execution stops, relevant only for training.
    # During the training we save the particles and configuration (including the current cache stage) in the Summary.pickle object.
    if configuration.training and (configuration.monitor and (configuration.monitoring_path != '')):
        B2ERROR("FEI-core: Custom Monitoring path is not allowed during training!")

    if configuration.cache is None:
        pickleName = 'Summary.pickle'
        if configuration.monitor:
            pickleName = os.path.join(configuration.monitoring_path, pickleName)

        if os.path.isfile(pickleName):
            particles_bkp, config_bkp = pickle.load(open(pickleName, 'rb'))
            # check if configuration changed
            for fd in configuration._fields:
                if fd == 'cache' or fd == 'roundMode':
                    continue
                if getattr(configuration, fd) != getattr(config_bkp, fd):
                    B2WARNING(
                        f"FEI-core: Configuration changed: {fd} from {getattr(config_bkp, fd)} to {getattr(configuration, fd)}")

            configuration = config_bkp
            cache = configuration.cache
            print("Cache: Replaced particles from steering and configuration from Summary.pickle: ", cache, configuration.roundMode)
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
    # For application we can run stage 0 to 7 at once
    stages = get_stages_from_particles(particles)

    # If the user provided a Summary.pickle file of a FEIv3 training we
    # convert the old weight files (with hashes), to the new naming scheme.
    # Afterwards the algorithm runs as usual
    if configuration.legacy is not None:
        convert_legacy_training(particles, configuration)

    # During the training we require the number of MC particles in the whole processed
    # data sample, because we don't want to write out billions of e.g. pion candidates.
    # Knowing the total amount of MC particles we can write out only every e.g. 10th candidate
    # That's why we have to write out the TrainingDataInformation before doing anything during the training phase.
    # During application we only need this if we run in monitor mode, and want to write out a summary in the end,
    # the summary contains efficiency, and the efficiency calculation requires the total number of MC particles.
    training_data_information = TrainingDataInformation(particles, outputPath=configuration.monitoring_path)
    if cache < 0 and configuration.training:
        print("Stage 0: Run over all files to count the number of events and McParticles")
        path.add_path(training_data_information.reconstruct())
        if configuration.training:
            save_summary(particles, configuration, 0)
        return FeiState(path, 0, [], [], [])
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
    # If the weight files for the PostReconstruction are not available for the current stage and we are in training mode,
    # we have to create the training data. The training itself is done by the do_trainings function which is called
    # either by the user after each step, or by the distributed.py script
    #
    # If the weight files for the PostReconstruction are not available for the current stage and we are not in training mode,
    # we keep going, as soon as the user will call process on the produced path he will get an error message that the
    # weight files are missing.
    #
    # Finally we keep track of the ParticleLists we use, so the user can run the RemoveParticles module to reduce the size of the
    # intermediate output of RootOutput.
    used_lists = []
    for stage, stage_particles in enumerate(stages):
        if len(stage_particles) == 0:
            print(f"Stage {stage}: No particles to reconstruct in this stage, skipping!")
            continue

        pre_reconstruction = PreReconstruction(stage_particles, configuration)
        post_reconstruction = PostReconstruction(stage_particles, configuration)

        if stage >= cache:
            print(f"Stage {stage}: PreReconstruct particles: ", [p.name for p in stage_particles])
            path.add_path(pre_reconstruction.reconstruct())
            if configuration.training and not (post_reconstruction.available() and configuration.roundMode == 0):
                print(f"Stage {stage}: Create training data for particles: ", [p.name for p in stage_particles])
                mc_counts = training_data_information.get_mc_counts()
                training_data = TrainingData(stage_particles, configuration, mc_counts)
                path.add_path(training_data.reconstruct())
                used_lists += [channel.name for particle in stage_particles for channel in particle.channels]
                break

        used_lists += [particle.identifier for particle in stage_particles]
        if (stage >= cache - 1) and not ((configuration.roundMode == 1) and configuration.training):
            if (configuration.roundMode == 3) and configuration.training:
                print(f"Stage {stage}: BDTs already applied for particles, no postReco needed: ", [p.name for p in stage_particles])
            else:
                print(f"Stage {stage}: Apply BDT for particles: ", [p.name for p in stage_particles])
                if configuration.training and not post_reconstruction.available():
                    raise RuntimeError("FEI-core: training of current stage was not successful, please retrain!")
                path.add_path(post_reconstruction.reconstruct())
            if (((configuration.roundMode == 2) or (configuration.roundMode == 3)) and configuration.training):
                break
    fsps_of_next_stages = [fsp for sublist in get_stages_from_particles(loader.get_fsp_lists())[stage+1:] for fsp in sublist]

    excludelists = []
    if configuration.training and (configuration.roundMode == 3):
        dontRemove = used_lists + fsps_of_next_stages
        # cleanup higher stages
        cleanup = basf2.register_module('RemoveParticlesNotInLists')
        print("FEI-REtrain: pruning basf2_input.root of higher stages")
        cleanup.param('particleLists', dontRemove)
        path.add_module(cleanup)

        # check which lists we have to exclude from the output
        import ROOT  # noqa
        excludedParticlesNonConjugated = [p.identifier for p in particles if p.identifier not in dontRemove]
        excludedParticles = [
            str(name) for name in list(
                ROOT.Belle2.ParticleListName.addAntiParticleLists(excludedParticlesNonConjugated))]
        root_file = ROOT.TFile.Open('basf2_input.root', "READ")
        tree = root_file.Get('tree')
        excludelists = []
        for branch in tree.GetListOfBranches():
            branchName = branch.GetName()
            if any(exParticle in branchName for exParticle in excludedParticles):
                excludelists.append(branchName)
        print("Exclude lists from output: ", excludelists)

    # If we run in monitor mode we are interested in the ModuleStatistics,
    # these statistics contain the runtime for each module which was run.
    if configuration.monitor:
        print("Add ModuleStatistics")
        output = basf2.register_module('RootOutput')
        output.param('outputFileName', os.path.join(configuration.monitoring_path, 'Monitor_ModuleStatistics.root'))
        output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size
        output.param('branchNamesPersistent', ['ProcessStatistics'])
        output.param('ignoreCommandLineOverride', True)
        path.add_module(output)

    # As mentioned above the FEI keeps track of the stages which are already reconstructed during the training
    # so we write out the Summary.pickle here, and increase the stage by one.
    if configuration.training or configuration.monitor:
        print("Save Summary.pickle")
        save_summary(particles, configuration, stage+1, pickleName=os.path.join(configuration.monitoring_path, 'Summary.pickle'))

    # Finally we return the path, the stage and the used lists to the user.
    return FeiState(path, stage+1, plists=used_lists, fsplists=fsps_of_next_stages, excludelists=excludelists)

# @endcond
