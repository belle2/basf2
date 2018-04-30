
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


def getOutputLFN(skimListName):

    outputLFN = skimListName

    if (skimListName == 'PRsemileptonicUntagged'):
        outputLFN = '11110100'
    if (skimListName == 'BottomoniumUpsilon'):
        outputLFN = '15440100'
    if (skimListName == 'BottomoniumEtabExclusive'):
        outputLFN = '15420100'
    if (skimListName == 'SLUntagged'):
        outputLFN = '11160200'
    if (skimListName == 'LeptonicUntagged'):
        outputLFN = '11130100'
    if (skimListName == 'BtoDh_hh'):
        outputLFN = '14140100'
    if (skimListName == 'BtoDh_Kshh'):
        outputLFN = '14140200'
    if (skimListName == 'BtoDh_Kspipipi0'):
        outputLFN = '14120400'
    if (skimListName == 'feiHadronicB0'):
        outputLFN = '11180100'
    if (skimListName == 'feiHadronicBplus'):
        outputLFN == '11180200'
    if (skimListName == 'feiSLB0WithOneLep'):
        outputLFN == '11180300'
    if (skimListName == 'feiSLBplusWithOneLep'):
        outputLFN == '11180400'
    if (skimListName == 'BtoXgamma'):
        outputLFN == '12160100'
    if (skimListName == 'BtoXll'):
        outputLFN == '12160200'

    return outputLFN


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
