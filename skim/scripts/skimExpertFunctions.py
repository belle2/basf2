# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Expert functions
~~~~~~~~~~~~~~~~

Some helper functions to do common tasks relating to skims.
Like testing, and for skim name encoding(decoding).
"""
from basf2 import create_path, register_module, LogLevel, B2ERROR, AfterConditionPath
from mdst import add_mdst_output
from modularAnalysis import removeParticlesNotInLists, skimOutputUdst, summaryOfLists
from skim.registry import skim_registry
import skimTestFilesInfo
import os
import sys
import inspect
import subprocess
import json
# For channels in fei skim
# from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


_all_skims = [name for _, name in skim_registry]


def encodeSkimName(skimScriptName):
    """
    Returns the appropriate 8 digit skim code that will be used as the output uDST
    file name for any give name of a skimming script.

    :param str skimScriptName: Name of the skim.
    """
    lookup_dict = {n: c for c, n in skim_registry}
    if skimScriptName not in lookup_dict:
        B2ERROR("Skim Unknown. Please add your skim to skimExpertFunctions.py.")
    return lookup_dict[skimScriptName]


def decodeSkimName(skimCode):
    """
    Returns the appropriate name of the skim given a specific skim code. This is useful to determine the skim script used
    to produce a specific uDST file, given the 8-digit code  name of the file itself.

    :param str code:
    """
    lookup_dict = {c: n for c, n in skim_registry}
    if skimCode not in lookup_dict:
        B2ERROR("Code Unknown. Please add your skim to skimExpertFunctions.py")
    return lookup_dict[skimCode]


def get_test_file(sample, skimCampaign):
    """
    Returns the KEKcc location of files used specifically for skim testing

    Arguments:
        sample: Type of MC sample: charged mixed ccbar uubar ddbar ssbar taupair or other of-resonance samples.
        skimCampaign: MC9, MC10, MC11, etc..
    """
    sampleName = skimCampaign + '_' + sample
    lookup_dict = {s: f for s, f in skimTestFilesInfo.kekcc_locations}
    if sampleName not in lookup_dict:
        B2ERROR("Testing file for this sample and skim campaign is not available.")
    return lookup_dict[sampleName]


def get_total_infiles(sample, skimCampaign):
    """
    Returns the total number of input Mdst files for a given sample. This is useful for resource estimate.
    Arguments:
        sample: Type of MC sample: charged mixed ccbar uubar ddbar ssbar taupair or other of-resonance samples.
        skimCampaign: MC9, MC10, MC11, etc..
    """
    sampleName = skimCampaign + '_' + sample
    lookup_dict = {s: f for s, f in skimTestFilesInfo.total_input_files}
    if sampleName not in lookup_dict:
        return 1000
    return lookup_dict[sampleName]


def add_skim(label, lists, path):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.

    Parameters:
        label (str): the registered skim name
        lists (list(str)): the list of ParticleList names that have been created by a skim list builder function
        path (basf2.Path): modules are added to this path

    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists, path=path)
    summaryOfLists(lists, path=path)


def setSkimLogging(path, additional_modules=[]):
    """
    Turns the log level to ERROR for  several modules to decrease
    the total size of the skim log files

    Parameters:
        skim_path (basf2.Path): modules are added to this path
        additional_modules (list(str)): an optional list of extra noisy module
            names that should be silenced
    """
    noisy_modules = ['ParticleLoader', 'ParticleVertexFitter'] + additional_modules
    for module in path.modules():
        if module.type() in noisy_modules:
            module.set_log_level(LogLevel.ERROR)
    return


def ifEventPasses(cut, conditional_path, path):
    """
    If the event passes the given ``cut`` proceed to process everything in ``conditional_path``.
    Afterwards return here and continue processing with the next module.

    Arguments:
        cut (str): selection criteria which needs to be fulfilled in order to continue with ``conditional_path``
        conditional_path (basf2.Path): path to execute if the event fulfills the criteria ``cut``
        path (basf2.Path): modules are added to this path
    """
    eselect = path.add_module("VariableToReturnValue", variable=f"passesEventCut({cut})")
    eselect.if_value('>=1', conditional_path, AfterConditionPath.CONTINUE)


def get_eventN(fileName):
    """
    Returns the number of events in a specific file

    Arguments:
     filename: Name of the file as clearly indicated in the argument's name.
    """

    process = subprocess.Popen(['b2file-metadata-show', '--json', fileName], stdout=subprocess.PIPE)
    out = process.communicate()[0]
    if process.returncode == 0:
        metadata = json.loads(out)
        nevents = metadata['nEvents']
        return nevents
    else:
        B2ERROR("FILE INVALID OR NOT FOUND.")


def skimOutputMdst(skimDecayMode, path=None, skimParticleLists=[], outputParticleLists=[], includeArrays=[], *,
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
    add_mdst_output(outputFile, path)
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)
