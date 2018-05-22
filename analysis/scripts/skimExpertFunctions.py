
# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This defines a function that outputs the  mdst outputs for skim testing purposes.
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


def encodeSkimName(skimListName):
    """Returns the skim code used in the output Udst file name."""
    skimCode = skimListName[0:7]

    if (skimListName == 'PRsemileptonicUntagged'):
        skimCode = '11110100'
    if (skimListName == 'BottomoniumUpsilon'):
        skimCode = '15440100'
    if (skimListName == 'BottomoniumEtabExclusive'):
        skimCode = '15420100'
    if (skimListName == 'SLUntagged'):
        skimCode = '11160200'
    if (skimListName == 'LeptonicUntagged'):
        skimCode = '11130300'
    if (skimListName == 'BtoDh_hh'):
        skimCode = '14140100'
    if (skimListName == 'BtoDh_Kspi0'):
        skimCode = '14120300'
    if (skimListName == 'BtoDh_Kshh'):
        skimCode = '14140200'
    if (skimListName == 'BtoDh_Kspipipi0'):
        skimCode = '14120400'
    if (skimListName == 'feiHadronicB0'):
        skimCode = '11180100'
    if (skimListName == 'feiHadronicBplus'):
        skimCode = '11180200'
    if (skimListName == 'feiSLB0WithOneLep'):
        skimCode = '11180300'
    if (skimListName == 'feiSLBplusWithOneLep'):
        skimCode = '11180400'
    if (skimListName == 'BtoXgamma'):
        skimCode = '12160100'
    if (skimListName == 'BtoXll'):
        skimCode = '12160200'
    if (skimListName == 'BtoPi0Pi0'):
        skimCode = '14120500'
    if (skimListName == 'Charm2BodyHadronic'):
        skimCode = '17240100'
    if (skimListName == 'Charm2BodyHadronicD0'):
        skimCode = '17230200'
    if (skimListName == 'Charm2BodyNeutrals'):
        skimCode = '17240300'
    if (skimListName == 'Charm2BodyNeutralsD0'):
        skimCode = '17230400'
    if (skimListName == 'Charm3BodyHadronic2'):
        skimCode = '17240500'
    if (skimListName == 'Charm3BodyHadronic'):
        skimCode = '17240600'
    if (skimListName == 'Charm3BodyHadronicD0'):
        skimCode = '17230700'
    if (skimListName == 'CharmRare'):
        skimCode = '17230800'
    if (skimListName == 'CharmSemileptonic'):
        skimCode = '17260900'
    if (skimListName == 'CharmlessHad'):
        skimCode = '14130100'
    if (skimListName == 'DoubleCharm'):
        skimCode = '14130200'
    if (skimListName == 'ISRpipicc'):
        skimCode = '16460100'
    if (skimListName == 'Systematics'):
        skimCode = '10600100'
    if (skimListName == 'SystematicsLambda'):
        skimCode = '10620200'
    if (skimListName == 'SystematicsTracking'):
        skimCode = '10600300'
    if (skimListName == 'Resonance'):
        skimCode = '10600400'
    if (skimListName == 'Tau'):
        skimCode = '18360100'
    if (skimListName == 'TCPV'):
        skimCode = '13160100'

    if (skimCode == skimListName[0:7]):
        B2FATAL("Skim unknown. Please add your skim to skimExpertFunctions.py!")

    return skimCode


def decodeSkimName(skimCode):
    """Returns the name of the skim given a skim code."""
    skimName = 'NoIdea'

    if (skimCode == '11110100'):
        skimName = 'PRsemileptonicUntagged'
    if (skimCode == '15440100'):
        skimName = 'BottomoniumUpsilon'
    if (skimCode == '15420100'):
        skimName = 'BottomoniumEtabExclusive'
    if (skimCode == '11160200'):
        skimName = 'SLUntagged'
    if (skimCode == '11130300'):
        skimName = 'LeptonicUntagged'
    if (skimCode == '14140100'):
        skimName = 'BtoDh_hh'
    if (skimCode == '14120300'):
        skimName = 'BtoDh_Kspi0'
    if (skimCode == '14140200'):
        skimName = 'BtoDh_Kshh'
    if (skimCode == '14120400'):
        skimName = 'BtoDh_Kspipipi0'
    if (skimCode == '11180100'):
        skimName = 'feiHadronicB0'
    if (skimCode == '11180200'):
        skimName = 'feiHadronicBplus'
    if (skimCode == '11180300'):
        skimName = 'feiSLB0WithOneLep'
    if (skimCode == '11180400'):
        skimName = 'feiSLBplusWithOneLep'
    if (skimCode == '12160100'):
        skimName = 'BtoXgamma'
    if (skimCode == '12160200'):
        skimName = 'BtoXll'
    if (skimCode == '14120500'):
        skimName = 'BtoPi0Pi0'
    if (skimCode == '17240100'):
        skimName = 'Charm2BodyHadronic'
    if (skimCode == '17230200'):
        skimName = 'Charm2BodyHadronicD0'
    if (skimCode == '17240300'):
        skimName = 'Charm2BodyNeutrals'
    if (skimCode == '17230400'):
        skimName = 'Charm2BodyNeutralsD0'
    if (skimCode == '17240500'):
        skimName = 'Charm3BodyHadronic2'
    if (skimCode == '17240600'):
        skimName = 'Charm3BodyHadronic'
    if (skimCode == '17230700'):
        skimName = 'Charm3BodyHadronicD0'
    if (skimCode == '17230800'):
        skimName = 'CharmRare'
    if (skimCode == '17260900'):
        skimName = 'CharmSemileptonic'
    if (skimCode == '14130100'):
        skimName = 'CharmlessHad'
    if (skimCode == '14130200'):
        skimName = 'DoubleCharm'
    if (skimCode == '16460100'):
        skimName = 'ISRpipicc'
    if (skimCode == '10600100'):
        skimName = 'Systematics'
    if (skimCode == '10620200'):
        skimName = 'SystematicsLambda'
    if (skimCode == '10600300'):
        skimName = 'SystematicsTracking'
    if (skimCode == '10600400'):
        skimName = 'Resonance'
    if (skimCode == '18360100'):
        skimName = 'Tau'
    if (skimCode == '13160100'):
        skimName = 'TCPV'

    if (skimName == 'NoIdea'):
        B2FATAL("Skim code  unknown. Please add your skim to skimExpertFunctions.py!")

    return skimName


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
