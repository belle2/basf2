# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Functions for skim testing and for skim name encoding.
"""

from basf2 import *
import os
import sys
import inspect
from vertex import *
from analysisPath import *
from modularAnalysis import *

# For channels in fei skim
from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


_skimNameMatching = [
    ('11110100', 'PRsemileptonicUntagged'),
    ('15440100', 'BottomoniumUpsilon'),
    ('15420100', 'BottomoniumEtabExclusive'),
    ('11160200', 'SLUntagged'),
    ('11130300', 'LeptonicUntagged'),
    ('14140100', 'BtoDh_hh'),
    ('14120300', 'BtoDh_Kspi0'),
    ('14140200', 'BtoDh_Kshh'),
    ('14120400', 'BtoDh_Kspipipi0'),
    ('11180100', 'feiHadronicB0'),
    ('11180200', 'feiHadronicBplus'),
    ('11180300', 'feiSLB0WithOneLep'),
    ('11180400', 'feiSLBplusWithOneLep'),
    ('12160100', 'BtoXgamma'),
    ('12160200', 'BtoXll'),
    ('14120500', 'BtoPi0Pi0'),
    ('17240100', 'Charm2BodyHadronic'),
    ('17230200', 'Charm2BodyHadronicD0'),
    ('17240300', 'Charm2BodyNeutrals'),
    ('17230400', 'Charm2BodyNeutralsD0'),
    ('17240500', 'Charm3BodyHadronic2'),
    ('17240600', 'Charm3BodyHadronic'),
    ('17230700', 'Charm3BodyHadronicD0'),
    ('17230800', 'CharmRare'),
    ('17260900', 'CharmSemileptonic'),
    ('19130100', 'CharmlessHad2Body'),
    ('19130200', 'CharmlessHad3Body'),
    ('14130200', 'DoubleCharm'),
    ('16460100', 'ISRpipicc'),
    ('10600100', 'Systematics'),
    ('10620200', 'SystematicsLambda'),
    ('10600300', 'SystematicsTracking'),
    ('10600400', 'Resonance'),
    ('10600500', 'SystematicsRadMuMu'),
    ('18360100', 'Tau'),
    ('13160100', 'TCPV'),
    ('18020300', 'ALP3Gamma'),
]


def encodeSkimName(skimScriptName):
    """ Returns the appropriate 8 digit skim code that will be used as the output uDST file name for any give name of a skimming script.
    :param str skimScriptName: Name of the skim.  """
    lookup_dict = {n: c for c, n in _skimNameMatching}
    if skimScriptName not in lookup_dict:
        B2ERROR("Skim Unknown. Please add your skim to skimExpertFunctions.py.")
    return lookup_dict[skimScriptName]


def decodeSkimName(skimCode):
    """ Returns the appropriate name of the skim given a specific skim code. This is useful to determine the skim script used
        to produce a specific uDST file, given the 8-digit code  name of the file itself.
    :param str code:
    """
    lookup_dict = {c: n for c, n in _skimNameMatching}
    if skimCode not in lookup_dict:
        B2ERROR("Code Unknown. Please add your skim to skimExpertFunctions.py")
    return lookup_dict[skimCode]


def setSkimLogging(skim_path=analysis_main, additional_modules=[]):
    """ Turns the log level to ERROR for  several modules to decrease the total size of the skim log files"""
    noisy_modules = ['ParticleLoader', 'ParticleVertexFitter'] + additional_modules
    for module in skim_path.modules():
        if module.type() in noisy_modules:
            module.set_log_level(LogLevel.ERROR)
    return


def skimOutputMdst(skimDecayMode, skimParticleLists=[], outputParticleLists=[], includeArrays=[], path=analysis_main, *,
                   outputFile=None, dataDescription=None):
    """
    Create a new path for events that contain a non-empty particle list specified via skimParticleLists.
    Write the accepted events as a mdst file, saving only particles from skimParticleLists
    and from outputParticleLists. It outputs a .mdst file.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    :param str skimDecayMode: Name of the skim. If no outputFile is given this is
        also the name of the output filename. This name will be added to the
        FileMetaData as an extra data description "skimDecayMode"
    :param list(str) skimParticleLists: Names of the particle lists to skim for.
        An event will be accepted if at least one of the particle lists is not empty
    :param list(str) outputParticleLists: Names of the particle lists to store in
        the output in addition to the ones in skimParticleLists
    :param list(str) includeArrays: datastore arrays/objects to write to the output
        file in addition to mdst and particle information
    :param basf2.Path path: Path to add the skim output to. Defaults to the default analysis path
    :param str outputFile: Name of the output file if different from the skim name
    :param dict dataDescription: Additional data descriptions to add to the output file. For example {"mcEventType":"mixed"}
    """

    # if no outputfile is specified, set it to the skim name
    if outputFile is None:
        outputFile = skimDecayMode

    # make sure the output filename has the correct extension
    if not outputFile.endswith(".mdst.root"):
        outputFile += ".mdst.root"

    skimfilter = register_module('SkimFilter')
    skimfilter.set_name('SkimFilter_' + skimDecayMode)
    skimfilter.param('particleLists', skimParticleLists)
    path.add_module(skimfilter)
    filter_path = create_path()
    skimfilter.if_value('=1', filter_path, AfterConditionPath.CONTINUE)

    # add_independent_path() is rather expensive, only do this for skimmed events
    skim_path = create_path()
    saveParticleLists = skimParticleLists + outputParticleLists
    removeParticlesNotInLists(saveParticleLists, path=skim_path)

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.setdefault("skimDecayMode", skimDecayMode)
    outputMdst(outputFile)
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)
