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
 Configuration classes

 The classes defined here are used to uniquely define a FEI training.
 Meaning:
  - The global configuration like database prefix, cache mode, monitoring, ... (FeiConfiguration)
  - The reconstructed Particles (Particle)
  - The reconstructed Channels of each particle (DecayChannel)
  - The MVA configuration for each channel (MVAConfiguration)
  - The Cut definitions of each channel (PreCutConfiguration)
  - The Cut definitions of each particle (PostCutConfiguration)
"""

import collections
import copy
import re
import itertools
import typing
import basf2

# Define classes at top level to make them pickable
# Creates new class via namedtuple, which are like a struct in C

FeiConfiguration = collections.namedtuple('FeiConfiguration',
                                          'prefix, cache, monitor, legacy, externTeacher, training, roundMode, monitoring_path')
FeiConfiguration.__new__.__defaults__ = ('FEI_TEST', None, True, None, 'basf2_mva_teacher', False, 0, '')
FeiConfiguration.__doc__ = "Fei Global Configuration class"
FeiConfiguration.prefix.__doc__ = "The database prefix used for all weight files"
FeiConfiguration.cache.__doc__ = "The stage which is passed as input, it is assumed that all previous stages"\
                                 " do not have to be reconstructed again. Can be either a number or"\
                                 " a filename containing a pickled number or"\
                                 " None in this case the environment variable FEI_STAGE is used."
FeiConfiguration.monitor.__doc__ = (
    "Determines the level of monitoring histograms to create. "
    "Set to False to disable monitoring. "
    "Set to 'simple' to enable lightweight histograms. "
    "Any other value will enable full monitoring histograms."
)
FeiConfiguration.legacy.__doc__ = "Pass the summary file of a legacy FEI training,"\
                                  " and the algorithm will be able to apply this training."
FeiConfiguration.externTeacher.__doc__ = "Teacher command e.g. basf2_mva_teacher, b2mva-kekcc-cluster-teacher"
FeiConfiguration.training.__doc__ = "If you train the FEI set this to True, otherwise to False"
FeiConfiguration.roundMode.__doc__ = "Round mode for the training. 0 default, 1 resuming, 2 finishing, 3 retraining."
FeiConfiguration.monitoring_path.__doc__ = "Path where monitoring histograms are stored."


MVAConfiguration = collections.namedtuple('MVAConfiguration', 'method, config, variables, target, sPlotVariable, spectators')
MVAConfiguration.__new__.__defaults__ = ('FastBDT',
                                         '--nTrees 400  --nCutLevels 10 --nLevels 3 --shrinkage 0.1 --randRatio 0.5',
                                         None, 'isSignal', None, {})
MVAConfiguration.__doc__ = "Multivariate analysis configuration class."
MVAConfiguration.method.__doc__ = "Method used by MVAInterface."
MVAConfiguration.config.__doc__ = "Method specific configuration string passed to basf2_mva_teacher"
MVAConfiguration.variables.__doc__ = "List of variables from the VariableManager."\
                                     " {} is expanded to one variable per daughter particle."
MVAConfiguration.target.__doc__ = "Target variable from the VariableManager."
MVAConfiguration.sPlotVariable.__doc__ = "Discriminating variable used by sPlot to do data-driven training."
MVAConfiguration.spectators.__doc__ = "Dictionary of spectator variables with their ranges from the VariableManager."


PreCutConfiguration = collections.namedtuple(
    'PreCutConfiguration', 'userCut, vertexCut, noBackgroundSampling,'
    'bestCandidateVariable, bestCandidateCut, bestCandidateMode, noSignalSampling, bkgSamplingFactor')
PreCutConfiguration.__new__.__defaults__ = ('', -2, False, None, 0, 'lowest', False, 1.0)
PreCutConfiguration.__doc__ = "PreCut configuration class. These cuts is employed before training the mva classifier."
PreCutConfiguration.userCut.__doc__ = "The user cut is passed directly to the ParticleCombiner."\
                                      " Particles which do not pass this cut are immediately discarded."
PreCutConfiguration.vertexCut.__doc__ = "The vertex cut is passed as confidence level to the VertexFitter."
PreCutConfiguration.noBackgroundSampling.__doc__ = "For very pure channels, the background sampling factor is too high" \
                                                   " and the MVA can't be trained. This disables background sampling."
PreCutConfiguration.bestCandidateVariable.__doc__ = "Variable from the VariableManager which is used to rank all candidates."
PreCutConfiguration.bestCandidateCut.__doc__ = "Number of best-candidates to keep after the best-candidate ranking."
PreCutConfiguration.bestCandidateMode.__doc__ = "Either lowest or highest."
PreCutConfiguration.noSignalSampling.__doc__ = "For channels with unknown br. frac., the signal sampling factor can be" \
                                               " overestimated and you loose signal samples in the training." \
                                               " This disables signal sampling."
PreCutConfiguration.bkgSamplingFactor.__doc__ = "Add additional multiplicative bkg. sampling factor, less than 1.0 to reduce."

PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value, bestCandidateCut')
PostCutConfiguration.__new__.__defaults__ = (0.0, 0)
PostCutConfiguration.__doc__ = "PostCut configuration class. This cut is employed after the training of the mva classifier."
PostCutConfiguration.value.__doc__ = "Absolute value used to cut on the SignalProbability of each candidate."
PostCutConfiguration.bestCandidateCut.__doc__ = "Number of best-candidates to keep, ranked by SignalProbability."

DecayChannel = collections.namedtuple(
    'DecayChannel',
    'name, label, decayString, daughters, mvaConfig, preCutConfig, decayModeID, pi0veto')
DecayChannel.__new__.__defaults__ = (None, None, None, None, None, None, None, False)
DecayChannel.__doc__ = "Decay channel of a Particle."
DecayChannel.name.__doc__ = "str:Name of the channel e.g. :code:`D0:generic_0`"
DecayChannel.label.__doc__ = "Label used to identify the decay channel e.g. for weight files independent of decayModeID"
DecayChannel.decayString.__doc__ = "DecayDescriptor of the channel e.g. D0 -> K+ pi-"
DecayChannel.daughters.__doc__ = "List of daughter particles of the decay channel e.g. [K+, pi-]"
DecayChannel.mvaConfig.__doc__ = "MVAConfiguration object which is used for this channel."
DecayChannel.preCutConfig.__doc__ = "PreCutConfiguration object which is used for this channel."
DecayChannel.decayModeID.__doc__ = "DecayModeID of this channel. Unique ID for each channel of this particle."
DecayChannel.pi0veto.__doc__ = "If true, additional pi0veto variables are added to the MVAs, useful only for decays with gammas."

MonitoringVariableBinning = {'mcErrors': ('mcErrors', 513, -0.5, 512.5),
                             'mcParticleStatus': ('mcParticleStatus', 257, -0.5, 256.5),
                             'dM': ('dM', 100, -1.0, 1.0),
                             'dQ': ('dQ', 100, -1.0, 1.0),
                             'abs(dM)': ('abs(dM)', 100, 0.0, 1.0),
                             'abs(dQ)': ('abs(dQ)', 100, 0.0, 1.0),
                             'pionID': ('pionID', 100, 0.0, 1.0),
                             'kaonID': ('kaonID', 100, 0.0, 1.0),
                             'protonID': ('protonID', 100, 0.0, 1.0),
                             'electronID': ('electronID', 100, 0.0, 1.0),
                             'muonID': ('muonID', 100, 0.0, 1.0),
                             'isSignal': ('isSignal', 2, -0.5, 1.5),
                             'isSignalAcceptMissingNeutrino': ('isSignalAcceptMissingNeutrino', 2, -0.5, 1.5),
                             'isPrimarySignal': ('isPrimarySignal', 2, -0.5, 1.5),
                             'chiProb': ('chiProb', 100, 0.0, 1.0),
                             'Mbc': ('Mbc', 100, 5.1, 5.4),
                             'cosThetaBetweenParticleAndNominalB': ('cosThetaBetweenParticleAndNominalB', 100, -10.0, 10.0),
                             'extraInfo(SignalProbability)': ('extraInfo(SignalProbability)', 100, 0.0, 1.0),
                             'extraInfo(decayModeID)': ('extraInfo(decayModeID)', 101, -0.5, 100.5),
                             'extraInfo(uniqueSignal)': ('extraInfo(uniqueSignal)', 2, -0.5, 1.5),
                             'extraInfo(preCut_rank)': ('extraInfo(preCut_rank)', 41, -0.5, 40.5),
                             'extraInfo(postCut_rank)': ('extraInfo(postCut_rank)', 41, -0.5, 40.5),
                             'daughterProductOf(extraInfo(SignalProbability))':
                             ('daughterProductOf(extraInfo(SignalProbability))', 100, 0.0, 1.0),
                             'pValueCombinationOfDaughters(extraInfo(SignalProbability))':
                             ('pValueCombinationOfDaughters(extraInfo(SignalProbability))', 100, 0.0, 1.0),
                             }


def variables2binnings(variables):
    """
    Convert given variables into a tuples which can be given to VariableToHistogram
    """
    return [MonitoringVariableBinning[v] if v in MonitoringVariableBinning else (v, 100, -10.0, 10.0) for v in variables]


def variables2binnings_2d(variables):
    """
    Convert given variables into a tuples which can be given to VariableToHistogram
    """
    result = []
    for v1, v2 in variables:
        b1 = MonitoringVariableBinning[v1] if v1 in MonitoringVariableBinning else (v1, 100, -10.0, 10.0)
        b2 = MonitoringVariableBinning[v2] if v2 in MonitoringVariableBinning else (v2, 100, -10.0, 10.0)
        result.append(b1 + b2)
    return result


def removeJPsiSlash(string: str) -> str:
    """
    Remove the / in the J/psi particle name
    """
    return string.replace('/', '')


class Particle:

    """
    The Particle class is the only class the end-user gets into contact with.
    The user creates an instance of this class for every particle he wants to reconstruct with the FEI algorithm,
    and provides MVAConfiguration, PreCutConfiguration and PostCutConfiguration. These can be overwritten per channel.
    """

    def __init__(self, identifier: str,
                 mvaConfig: MVAConfiguration,
                 preCutConfig: PreCutConfiguration = PreCutConfiguration(),
                 postCutConfig: PostCutConfiguration = PostCutConfiguration()):
        """
        Creates a Particle without any decay channels. To add decay channels use addChannel method.
            @param identifier is the pdg name of the particle as a string
                   with an optional additional user label separated by ':'
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig intermediate pre cut configuration
            @param postCutConfig post cut configuration
        """
        #: pdg name of the particle with an optional additional user label separated by :
        self.identifier = identifier + ':generic' if len(identifier.split(':')) < 2 else identifier
        v = self.identifier.split(':')
        #: The name of the particle as correct pdg name e.g. K+, pi-, D*0.
        self.name = v[0]
        #: Additional label like hasMissing or has2Daughters
        self.label = v[1]
        #: multivariate analysis configuration (see MVAConfiguration)
        self.mvaConfig = mvaConfig
        #: DecayChannel objects added by addChannel() method.
        self.channels = []
        #: intermediate cut configuration (see PreCutConfiguration)
        self.preCutConfig = preCutConfig
        #: post cut configuration (see PostCutConfiguration)
        self.postCutConfig = postCutConfig

    def __eq__(self, a):
        """
        Compares to Particle objects.
        They are equal if their identifier, name, label, all channels, preCutConfig and postCutConfig is equal
        @param a another Particle object
        """
        return (self.identifier == a.identifier and self.name == a.name and self.label == a.label and
                self.channels == a.channels and self.preCutConfig == a.preCutConfig and self.postCutConfig == a.postCutConfig)

    def __str__(self):
        """
        Creates a string representation of a Particle object.
        """
        return str((self.identifier, self.channels, self.preCutConfig, self.postCutConfig, self.mvaConfig))

    def __hash__(self):
        """
        Creates a hash of a Particle object.
        This is necessary to use this as a key in a dictionary
        """
        return hash((self.identifier, self.channels, self.preCutConfig, self.postCutConfig, self.mvaConfig))

    @property
    def daughters(self):
        """ Property returning list of unique daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    def addChannel(self,
                   daughters: typing.Sequence[str],
                   mvaConfig: MVAConfiguration = None,
                   preCutConfig: PreCutConfiguration = None,
                   pi0veto: bool = False):
        """
        Appends a new decay channel to the Particle object.
            @param daughters is a list of pdg particle names e.g. ['pi+','K-']
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig pre cut configuration object
            @param pi0veto if true, additional pi0veto variables are added to the MVA configuration
        """
        # Append generic label to all defined daughters if no label was set yet
        daughters = [d + ':generic' if ':' not in d else d for d in daughters]
        # Use default mvaConfig of this particle if no channel-specific config is given
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        # Use default preCutConfig of this particle if no channel-specific config is given
        preCutConfig = copy.deepcopy(self.preCutConfig if preCutConfig is None else preCutConfig)
        # At the moment all channels must have the same target variable. Why?
        if mvaConfig is not None and mvaConfig.target != self.mvaConfig.target:
            basf2.B2FATAL(
                f'Particle {self.identifier} has common target {self.mvaConfig.target}, while channel '
                f'{" ".join(daughters)} has {mvaConfig.target}. Each particle must have exactly one target!')
        # Replace generic-variables with ordinary variables.
        # All instances of {} are replaced with all combinations of daughter indices
        mvaVars = []
        for v in mvaConfig.variables:
            if v.count('{') == 0:
                mvaVars.append(v)
                continue
            matches = re.findall(r'\{\s*\d*\s*\.\.\s*\d*\s*\}', v)
            if len(matches) == 0 and v.count('{}') == 0:
                mvaVars.append(v)
            elif v.count('{}') > 0 and len(matches) > 0:
                basf2.B2FATAL(f'Variable {v} contains both '+'{}'+f' and {matches}. Only one is allowed!')
            elif len(matches) > 0:
                ranges = []
                skip = False
                for match in matches:
                    tempRange = match[1:-1].split('..')
                    if tempRange[0] == '':
                        tempRange[0] = 0
                    else:
                        tempRange[0] = int(tempRange[0])
                        if tempRange[0] >= len(daughters):
                            basf2.B2DEBUG(11, f'Variable {v} contains index {tempRange[0]} which is more than daughters, skipping!')
                            skip = True
                            break
                    if tempRange[1] == '':
                        tempRange[1] = len(daughters)
                    else:
                        tempRange[1] = int(tempRange[1])
                        if tempRange[1] > len(daughters):
                            basf2.B2DEBUG(11, f'Variable {v} contains index {tempRange[1]} which is more than daughters, skipping!')
                            skip = True
                            break
                    ranges.append(tempRange)
                if skip:
                    continue
                if len(ranges) == 1:
                    mvaVars += [v.replace(matches[0], str(c)) for c in range(ranges[0][0], ranges[0][1])]
                else:
                    for match in matches:
                        v = v.replace(match, '{}')
                    mvaVars += [v.format(*c) for c in itertools.product(*[range(r[0], r[1]) for r in ranges])]
            elif v.count('{}') <= len(daughters):
                mvaVars += [v.format(*c) for c in itertools.combinations(list(range(0, len(daughters))), v.count('{}'))]
            elif v.count('{}') > len(daughters):
                basf2.B2DEBUG(11, f'Variable {v} contains more brackets than daughters, which is why it will be ignored!')
                continue
            else:
                basf2.B2FATAL(f'Something went wrong with variable {v}!')
        mvaConfig = mvaConfig._replace(variables=mvaVars)
        # Add new channel
        decayModeID = len(self.channels)
        self.channels.append(DecayChannel(name=self.identifier + '_' + str(decayModeID),
                                          label=removeJPsiSlash(self.identifier + ' ==> ' + ' '.join(daughters)),
                                          decayString=self.identifier + '_' + str(decayModeID) + ' -> ' + ' '.join(daughters),
                                          daughters=daughters,
                                          mvaConfig=mvaConfig,
                                          preCutConfig=preCutConfig,
                                          decayModeID=decayModeID,
                                          pi0veto=pi0veto))
        return self

# @endcond
