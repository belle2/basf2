# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Functions for skim testing and for skim name encoding.
"""
from modularAnalysis import *
from basf2 import *
import os
import sys
import inspect
from analysisPath import *
import subprocess
import json
# For channels in fei skim
# from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


all_skims = [
    "Dark", "BtoCharmless", "BtoCharm", "ALP3Gamma",
    "BottomoniumEtabExclusive", "BottomoniumUpsilon", "TauGeneric",
    "SystematicsRadMuMu", "SystematicsRadEE", "LFVZpInvisible", "LFVZpVisible",
    "SinglePhotonDark", "SystematicsTracking", "BottomoniumUpsilon",
    "BottomoniumEtabExclusive", "SystematicsLambda", "Systematics",
    "Resonance", "ISRpipicc", "BtoDh_Kspipipi0", "BtoPi0Pi0",
    "CharmSemileptonic", "BottomoniumEtabExclusive", "BottomoniumUpsilon",
    "feiSLB0WithOneLep", "feiBplusWithOneLep", "feiHadronicB0",
    "feiHadronicBplus", "BtoPi0Pi0", "Charm3BodyHadronic2",
    "Charm3BodyHadronic", "Charm3BodyHadronicD0", "Charm2BodyHadronic",
    "Charm2BodyNeutrals", "Charm2BodyNeutralsD0", "BtoDh_Kspi0", "BtoDh_hh",
    "BtoDh_Kshh", "Tau", "PRsemileptonicUntagged", "SLUntagged",
    "LeptonicUntagged", "TCPV", "CharmRare", "BtoXll", "BtoXgamma", "TauLFV",
]


_total_input_files = {
    ('MC9_mixedBGx1', 3564),
    ('MC9_chargededBGx1', 3770),
    ('MC9_uubarBGx1', 6115),
    ('MC9_ddbarBGx1', 1783),
    ('MC9_ssbarBGx1', 1704),
    ('MC9_ccbarBGx1', 7088),
    ('MC9_taupairBGx1', 3501),
    ('MC9_mixedBGx0', 357),
    ('MC9_chargededBGx0', 377),
    ('MC9_uubarBGx0', 803),
    ('MC9_ddbarBGx0', 201),
    ('MC9_ssbarBGx0', 192),
    ('MC9_ccbarBGx0', 760),
    ('MC9_taupairBGx0', 368),
}
_test_file_list = {
    ('MC10_mixedBGx1', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004770/s00/e0000/4S/r00000/mixed/' +
     'mdst/sub00/mdst_000001_prod00004770_task00000001.root'),
    ('MC10_mixedBGx0', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003591/s00/e0000/4S/r00000/mixed/' +
     'mdst/sub00/mdst_000001_prod00003591_task00000001.root'),
    ('MC10_chargedBGx1', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004771/s00/e0000/4S/r00000/charged/' +
     'mdst/sub00/mdst_000001_prod00004771_task00000001.root'),
    ('MC10_chargedBGx0', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003593/s00/e0000/4S/r00000/charged/' +
     'mdst/sub00/mdst_000001_prod00003593_task00000001.root'),
    ('MC10_uubarBGx1', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004772/s00/e0000/4S/r00000/uubar/' +
     'mdst/sub00/mdst_000001_prod00004772_task00000001.root'),
    ('MC10_uubarBGx0', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003595/s00/e0000/4S/r00000/uubar/' +
     'mdst/sub00/mdst_000001_prod00003595_task00000001.root'),
    ('MC10_ccbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004775/s00/e0000/4S/r00000/ccbar/' +
     'mdst/sub00/mdst_000001_prod00004775_task00000001.root'),
    ('MC10_ccbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003601/s00/e0000/4S/r00000/ccbar/' +
     'mdst/sub00/mdst_000001_prod00003601_task00000001.root'),
    ('MC10_ddbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004773/s00/e0000/4S/r00000/ddbar/' +
     'mdst/sub00/mdst_000001_prod00004773_task00000001.root'),
    ('MC10_ddbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003597/s00/e0000/4S/r00000/ddbar/' +
     'mdst/sub00/mdst_000001_prod00003597_task00000001.root'),
    ('MC10_ssbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004774/s00/e0000/4S/r00000/ssbar/' +
     'mdst/sub00/mdst_000001_prod00004774_task00000001.root'),
    ('MC10_ssbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003599/s00/e0000/4S/r00000/ssbar/' +
     'mdst/sub00/mdst_000001_prod00003599_task00000001.root'),
    ('MC10_taupairBGx1', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004776/s00/e0000/4S/r00000/taupair/' +
     'mdst/sub00/mdst_000001_prod00004776_task00000001.root'),
    ('MC10_taupairBGx0', '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003603/s00/e0000/4S/r00000/taupair/' +
     'mdst/sub00/mdst_000001_prod00003603_task00000001.root'),
    ('MC9_mixedBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
     'mdst_000001_prod00002288_task00000001.root'),
    ('MC9_chargedBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002289/e0000/4S/r00000/charged/sub00/' +
     'mdst_000001_prod00002289_task00000001.root'),
    ('MC9_ccbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002321/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_000001_prod00002321_task00000001.root'),
    ('MC9_ssbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002312/e0000/4S/r00000/ssbar/sub00/' +
     'mdst_000001_prod00002312_task00000001.root'),
    ('MC9_uubarBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002290/e0000/4S/r00000/uubar/sub00/' +
     'mdst_000001_prod00002290_task00000001.root'),
    ('MC9_ddbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002311/e0000/4S/r00000/ddbar/sub00/' +
     'mdst_000268_prod00002311_task00000268.root'),
    ('MC9_taupairBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002322/e0000/4S/r00000/taupair/sub00/' +
     'mdst_000001_prod00002322_task00000001.root'),
    ('MC9_mixedBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002166/e0000/4S/r00000/mixed/sub00/' +
     'mdst_000001_prod00002166_task00000001.root'),
    ('MC9_chargedBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002167/e0000/4S/r00000/charged/sub00/' +
     'mdst_000001_prod00002167_task00000001.root'),
    ('MC9_ccbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002171/e0000/4S/r00000/ccbar/sub00/' +
     'mdst_000001_prod00002171_task00000001.root'),
    ('MC9_ssbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002170/e0000/4S/r00000/ssbar/sub00/' +
     'mdst_000001_prod00002170_task00000001.root'),
    ('MC9_uubarBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002168/e0000/4S/r00000/uubar/sub00/' +
     'mdst_000001_prod00002168_task00000001.root'),
    ('MC9_ddbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002169/e0000/4S/r00000/ddbar/sub00/' +
     'mdst_000001_prod00002169_task00000001.root'),
    ('MC9_taupairBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002172/e0000/4S/r00000/taupair/sub00/' +
     'mdst_000001_prod00002172_task00000001.root'),
    ('MC9_ccbar3SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002709/e0000/3S/r00000/ccbar/sub00/' +
     'mdst_000001_prod00002709_task00000001.root'),
    ('MC9_generic3SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002705/e0000/3S/r00000/generic/sub00/' +
     'mdst_000001_prod00002705_task00000001.root'),
    ('MC9_ddbar3SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002707/e0000/3S/r00000/ddbar/sub00/' +
     'mdst_000001_prod00002707_task00000001.root'),
    ('MC9_ssbar3SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002708/e0000/3S/r00000/ssbar/sub00/' +
     'mdst_000001_prod00002708_task00000001.root'),
    ('MC9_taupair3SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002714/e0000/3S/r00000/taupair/sub00/' +
     'mdst_000001_prod00002714_task00000001.root'),
    ('MC9_uubar3SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002706/e0000/3S/r00000/uubar/sub00/' +
     'mdst_000001_prod00002706_task00000001.root'),
    # 5S samples
    ('MC9_bsbs5SBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002281/e0000/5S/r00000/bsbs/sub00/' +
     'mdst_000002_prod00002281_task00000002.root'),
    ('MC9_nonbsbs5SBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002282/e0000/5S/r00000/nonbsbs/sub00/' +
     'mdst_000002_prod00002282_task00000002.root'),
    ('MC9_uubar5SBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002283/e0000/5S/r00000/uubar/sub00/' +
     'mdst_000001_prod00002283_task00000001.root'),
    ('MC9_ddbar5SBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002284/e0000/5S/r00000/ddbar/sub00/' +
     'mdst_000001_prod00002284_task00000001.root'),
    ('MC9_ssbar5SBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002285/e0000/5S/r00000/ssbar/sub00/' +
     'mdst_000001_prod00002285_task00000001.root'),
    ('MC9_ccbar5SBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002286/e0000/5S/r00000/ccbar/sub00/' +
     'mdst_000001_prod00002286_task00000001.root'),
    ('MC9_taupair5SBGx0', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002287/e0000/5S/r00000/taupair/sub00/' +
     'mdst_000001_prod00002287_task00000001.root'),
    ('MC9_bsbs5SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002664/e0000/5S/r00000/bsbs/sub00/' +
     'mdst_000001_prod00002664_task00000001.root'),
    ('MC9_nonbsbs5SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002665/e0000/5S/r00000/nonbsbs/sub00/' +
     'mdst_000001_prod00002665_task00000001.root'),
    ('MC9_uubar5SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002667/e0000/5S/r00000/uubar/sub00/' +
     'mdst_000001_prod00002667_task00000001.root'),
    ('MC9_ddbar5SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002669/e0000/5S/r00000/ddbar/sub00/' +
     'mdst_000001_prod00002669_task00000001.root'),
    ('MC9_ssbar5SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002670/e0000/5S/r00000/ssbar/sub00/' +
     'mdst_000001_prod00002670_task00000001.root'),
    ('MC9_ccbar5SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002671/e0000/5S/r00000/ccbar/sub00/' +
     'mdst_000001_prod00002671_task00000001.root'),
    ('MC9_taupair5SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002698/e0000/5S/r00000/taupair/sub00/' +
     'mdst_000001_prod00002698_task00000001.root'),
    # 6S samples
    ('MC9_bsbs6SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002715/e0000/6S/r00000/bsbs/sub00/' +
     'mdst_000001_prod00002715_task00000001.root'),
    ('MC9_nonbsbs6SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002716/e0000/6S/r00000/nonbsbs/sub00/' +
     'mdst_000001_prod00002716_task00000001.root'),
    ('MC9_ccbar6SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002702/e0000/6S/r00000/ccbar/sub00/' +
     'mdst_000001_prod00002702_task00000001.root'),
    ('MC9_uubar6SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002699/e0000/6S/r00000/uubar/sub00/' +
     'mdst_000001_prod00002699_task00000001.root'),
    ('MC9_ddbar6SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002700/e0000/6S/r00000/ddbar/sub00/' +
     'mdst_000001_prod00002700_task00000001.root'),
    ('MC9_ssbar6SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002701/e0000/6S/r00000/ssbar/sub00/' +
     'mdst_000001_prod00002701_task00000001.root'),
    ('MC9_taupair6SBGx1', '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002703/e0000/6S/r00000/taupair/sub00/' +

     'mdst_000001_prod00002703_task00000001.root'),
}

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
    ('17240100', 'Charm2BodyHadronic'),  # D* -> D0 -> K pi/pi pi/K K
    ('17240200', 'Charm3BodyHadronic'),  # D* -> D0 -> K- pi+ pi0 (RS+WS)
    ('17240300', 'Charm3BodyHadronic1'),  # D* -> D0 -> h h pi0
    ('17240400', 'Charm2BodyNeutrals2'),  # D* -> D0 -> Ks omega / Ks eta -> Ks pi+ pi- pi0
    ('17240500', 'Charm3BodyHadronic3'),  # D* -> D0 -> K- pi+ eta (RS+WS)
    ('17240600', 'Charm2BodyNeutrals'),  # D* -> D0 -> pi0 pi0/Ks pi0/Ks Ks
    ('17240700', 'Charm3BodyHadronic2'),  # D* -> D0 -> h h Ks
    ('17230100', 'Charm2BodyHadronicD0'),  # D0 -> K pi/pi pi/K K
    ('17230200', 'Charm2BodyNeutralsD0'),  # D0 -> pi0 pi0/Ks pi0/Ks Ks
    ('17230300', 'CharmRare'),  # D0 -> g g/e e/mu mu
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
    ('10600600', 'SystematicsEELL'),
    ('10600700', 'SystematicsRadEE'),
    ('18360100', 'TauLFV'),
    ('13160100', 'TCPV'),
    ('18020100', 'SinglePhotonDark'),
    ('18020300', 'ALP3Gamma'),
    ('18520400', 'LFVZpInvisible'),
    ('18520500', 'LFVZpVisible'),
    ('18570600', 'TauGeneric'),
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


def get_test_file(sample, skimCampaign):
    """
    Returns the KEKcc location of files used specifically for skim testing

    Arguments:
        sample: Type of MC sample: charged mixed ccbar uubar ddbar ssbar taupair or other of-resonance samples.
        skimCampaign: MC9, MC10, MC11, etc..
    """
    sampleName = skimCampaign + '_' + sample
    lookup_dict = {s: f for s, f in _test_file_list}
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
    lookup_dict = {s: f for s, f in _total_input_files}
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


def setSkimLogging(path=analysis_main, additional_modules=[]):
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


def ifEventPasses(cut, conditional_path, path=analysis_main):
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
