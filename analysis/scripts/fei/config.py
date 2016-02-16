#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016
#
# Configuration classes
#

import collections
import copy
import itertools
import typing

# Define classes at top level to make them pickable
# Creates new classs via namedtuple, which are like a struct in C

MVAConfiguration = collections.namedtuple('MVAConfiguration', 'name, type, config, variables, target, sPlotVariable')
MVAConfiguration.__new__.__defaults__ = ('FastBDT', 'Plugin',
                                         '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
                                         None, 'isSignal', None)
MVAConfiguration.__doc__ = "Multivariate analysis configuration class."
MVAConfiguration.name.__doc__ = "Name of the TMVA Method."
MVAConfiguration.type.__doc__ = "Type of the TMVA Method (e.g. Plugin)."
MVAConfiguration.config.__doc__ = "Configuration string of method apsed to TMVA."
MVAConfiguration.variables.__doc__ = "List of variables from the VariableManager."\
                                     "{} is expanded to one variable per daughter particle."
MVAConfiguration.target.__doc__ = "Target variable from the VariableManager."
MVAConfiguration.sPlotVariable.__doc__ = "Discriminating variable used by sPlot to do data-driven training."


PreCutConfiguration = collections.namedtuple('PreCutConfiguration', 'userCut, vertexCut, bestCandidateVariable, '
                                             'bestCandidateCut, bestCandidateMode')
PreCutConfiguration.__new__.__defaults__ = ('', -2, None, 0, 'lowest')
PreCutConfiguration.__doc__ = "PreCut configuration class. These cuts is employed before training the mva classifier."
PreCutConfiguration.userCut.__doc__ = "The user cut is passed directly to the ParticleCombiner."\
                                      "Particles which do not pass this cut are immediatly discarded."
PreCutConfiguration.vertexCut.__doc__ = "The vertex cut is passed as confidence level to the VertexFitter."
PreCutConfiguration.bestCandidateVariable.__doc__ = "Variable from the VariableManager which is used to rank all candidates."
PreCutConfiguration.bestCandidateMode.__doc__ = "Either lowest or highest."
PreCutConfiguration.bestCandidateCut.__doc__ = "Number of best-candidates to keep after the best-candidate ranking."

PostCutConfiguration = collections.namedtuple('PostCutConfiguration', 'value, bestCandidateCut')
PostCutConfiguration.__new__.__defaults__ = (0.0, 0)
PostCutConfiguration.__doc__ = "PostCut configuration class. This cut is employed after the training of the mva classifier."
PostCutConfiguration.value.__doc__ = "Absolute value used to cut on the SignalProbability of each candidate."
PostCutConfiguration.bestCandidateCut.__doc__ = "Number of best-candidates to keep, ranked by SignalProbability."

DecayChannel = collections.namedtuple('DecayChannel', 'name, daughters, mvaConfig, preCutConfig, decayModeID')
DecayChannel.__doc__ = "Decay channel of a Particle."
DecayChannel.name.__doc__ = "DecayDescriptor of the channel e.g. D0 ==> K+ pi-"
DecayChannel.daughters.__doc__ = "List of daughter particles of the decay channel e.g. [K+, pi-]"
DecayChannel.mvaConfig.__doc__ = "MVAConfiguration object which is used for this channel."
DecayChannel.preCutConfig.__doc__ = "PreCutConfiguration object which is used for this channel."
DecayChannel.decayModeID.__doc__ = "DecayModeID of this channel. Unique ID for each channel of this particle."


MonitoringVariableBinning = {'mcErrors': ('mcErrors', 513, -0.5, 512.5),
                             'mcParticleStatus': ('mcParticleStatus', 257, -0.5, 256.5),
                             'dM': ('dM', 100, -1.0, 1.0),
                             'dQ': ('dQ', 100, -1.0, 1.0),
                             'abs(dM)': ('abs(dM)', 100, 0.0, 1.0),
                             'abs(dQ)': ('abs(dQ)', 100, 0.0, 1.0),
                             'piid': ('piid', 100, 0.0, 1.0),
                             'Kid': ('Kid', 100, 0.0, 1.0),
                             'prid': ('prid', 100, 0.0, 1.0),
                             'eid': ('eid', 100, 0.0, 1.0),
                             'muid': ('muid', 100, 0.0, 1.0),
                             'isSignal': ('isSignal', 2, -0.5, 1.5),
                             'isSignalAcceptMissingNeutrino': ('isSignalAcceptMissingNeutrino', 2, -0.5, 1.5),
                             'isPrimarySignal': ('isPrimarySignal', 2, -0.5, 1.5),
                             'chiProb': ('chiProb', 100, 0.0, 1.0),
                             'Mbc': ('Mbc', 100, 5.1, 5.4),
                             'cosThetaBetweenParticleAndTrueB': ('cosThetaBetweenParticleAndTrueB', 100, -10.0, 10.0),
                             'extraInfo(SignalProbability)': ('extraInfo(SignalProbability)', 100, 0.0, 1.0),
                             'extraInfo(uniqueSignal)': ('extraInfo(uniqueSignal)', 2, -0.5, 1.5),
                             'extraInfo(preCut_rank)': ('extraInfo(preCut_rank)', 41, -0.5, 40.5),
                             'extraInfo(postCut_rank)': ('extraInfo(postCut_rank)', 41, -0.5, 40.5),
                             'daughterProductOf(extraInfo(SignalProbability))':
                             ('daughterProductOf(extraInfo(SignalProbability))', 100, 0.0, 1.0),
                             }


def variables2binnings(variables):
    """
    Convert given variables into a tuples which can be given to VariableToHistogram
    """
    return [MonitoringVariableBinning[v] if v in MonitoringVariableBinning else (v, 100, -10.0, 10.0) for v in variables]


class Particle(object):

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
                   with an optional additional user label seperated by ':'
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig intermediate pre cut configuration
            @param postCutConfig post cut configuration
        """
        #: pdg name of the particle with an optional additional user label seperated by :
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
        return (self.identifier == a.identifier and self.name == a.name and self.label == a.label and
                self.channels == a.channels and self.preCutConfig == a.preCutConfig and self.postCutConfig == a.postCutConfig)

    def __str__(self):
        return str((self.identifier, self.channels, self.preCutConfig, self.postCutConfig, self.mvaConfig))

    def __hash__(self):
        return hash((self.identifier, self.channels, self.preCutConfig, self.postCutConfig, self.mvaConfig))

    @property
    def daughters(self):
        """ Property returning list of unique daughter particles of all channels """
        return list(frozenset([daughter for channel in self.channels for daughter in channel.daughters]))

    def addChannel(self,
                   daughters: typing.Sequence[str],
                   mvaConfig: MVAConfiguration = None,
                   preCutConfig: PreCutConfiguration = None):
        """
        Appends a new decay channel to the Particle object.
            @param daughters is a list of pdg particle names e.g. ['pi+','K-']
            @param mvaConfig multivariate analysis configuration
            @param preCutConfig pre cut configuration object
        """
        # Append generic label to all defined daughters if no label was set yet
        daughters = [d + ':generic' if ':' not in d else d for d in daughters]
        # Use default mvaConfig of this particle if no channel-specific config is given
        mvaConfig = copy.deepcopy(self.mvaConfig if mvaConfig is None else mvaConfig)
        # Use default preCutConfig of this particle if no channel-specific config is given
        preCutConfig = copy.deepcopy(self.preCutConfig if preCutConfig is None else preCutConfig)
        # At the moment all channels must have the same target variable. Why?
        if mvaConfig is not None and mvaConfig.target != self.mvaConfig.target:
            B2FATAL(
                'Particle %s has common target %s, while channel %s has %s. Each particle must have exactly one target!' %
                (particle.identifier, self.mvaConfig.target, ' '.join(daughters), mvaConfig.target))
        # Replace generic-variables with ordinary variables.
        # All instances of {} are replaced with all combinations of daughter indices
        mvaVars = []
        for v in mvaConfig.variables:
            if v.count('{}') <= len(daughters):
                mvaVars += [v.format(*c) for c in itertools.combinations(list(range(0, len(daughters))), v.count('{}'))]
        mvaConfig = mvaConfig._replace(variables=mvaVars)
        # Add new channel
        self.channels.append(DecayChannel(name=self.identifier + ' ==> ' + ' '.join(daughters),
                                          daughters=daughters,
                                          mvaConfig=mvaConfig,
                                          preCutConfig=preCutConfig,
                                          decayModeID=len(self.channels)))
        return self
