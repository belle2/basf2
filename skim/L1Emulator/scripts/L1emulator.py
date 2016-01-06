#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_L1Emulation(path):
    """
    This function adds the L1Emulator module to a path
    """

    # The first four trigger pathes are for eclBhabhaVetoHLT, BhabhaVetoHLT, SBhabhaVetoHLT, and ggVetoHLT,
    # respectivley,which have to be defined sequentially.The corresponding
    # logics with names (eclBhabhaVetoHLT, BhabhaVetoHLT, SBhabhaVetoHLT, ggVetoHLT) are
    # available in other triggers.
    L1Emulation_Veto(path)

    # After definition, the logics "eclBhabhaVetoHLT, BhabhaVetoHLT,
    # SBhabhaVetoHLT, and ggVetoHLT" couldbe used in the following trigger.
    # Define users' triggers. The examples are following.
    # BhabhaAccept1 and BhabhaAccept2
    L1Emulation('BhabhaAccept1', 'NtHLT>=1 and NtcHLT>=1 and eclBhabhaVetoHLT==1', path,
                [1700, 1700, 600, 300, 100, 80, 50, 30, 30, 20, 15, 10, 10, 10, 10])

    L1Emulation('BhabhaAccept2', 'NtHLT>=2 and NtcHLT>=1 and maxAngTTCMSHLT>2.618 and [EC1HLT>3.0 or EC2HLT>1.0]', path, [
                1700, 1700, 600, 300, 100, 80, 50, 30, 30, 20, 15, 10, 10, 10, 10])

    # ggAccept
    L1Emulation('ggAccept1', 'eclBhabhaVetoHLT==1 and BhabhaVetoHLT==0', path, [10])

    # single leg gamma trigger
    L1Emulation('ggAccept2', 'NcHLT>=1 and [ET1HLT==-1 or ET2HLT==-1] and [EC1HLT>3.0 or EC2HLT>1.0]', path, [20])

    # two tracks
    # standard two tracks trigger
    L1Emulation('TwoTracks', 'NltHLT==2 and BhabhaVetoHLT==0', path, [1])

    # two track without veto
    L1Emulation('TwoTracksScale', 'NltHLT==2', path, [2000])

    # one track one muon
    L1Emulation('OneTrkOneMu', 'NtHLT>=1 and Layer1KLMHLT>=3 and maxAngTMHLT>0.785', path, [1])

    # two muons
    # L1Emulation('TwoMu','Layer1KLMHLT>=3 and Layer2KLMHLT>=3 and maxAngMMHLT>45',path,[10])

    # single muon no ECL
    L1Emulation('SingleMu', 'NtHLT>=1 and [Layer1KLMHLT>=4 or Layer2KLMHLT>=4]', path, [1])

    # single mun no KLM
    # L1Emulation('SingleMu','[[ET1HLT>0 and ET1HLT<0.5] or [ET2HLT>0 and ET2HLT<0.5]] and BhabhaVetoHLT==0',path,[10])

    # Tau 1 vs 1
    # one track one cluster
    L1Emulation(
        'OneTrkOneCluster',
        'NcHLT>=1 and EC1HLT>0.5 and NtHLT>=1 and AngGTHLT>0.785 and BhabhaVetoHLT==0 and SBhabhaVetoHLT==0',
        path,
        [1])

    # one track one cluster
    L1Emulation('OneTrkOneClusterScale', 'NcHLT>=1 and EC1HLT>0.5 and NtHLT>=1 and AngGTHLT>0.785', path, [2000])

    # two back to back tracks
    # L1Emulation('TwoBBtracks','NtHLT==2 and maxAngTTHLT>45 and BhabhaVetoHLT==0',path,[1])

    # two back to back tracks no veto
    # L1Emulation('TwoBBtracks','NtHLT==2 and maxAngTTHLT>45',path,[2000])

    # back to back cluster
    # L1Emulation('BBCluster','NcHLT>=2 and EC1HLT>0.5 and EC2HLT>0.5
    # and AngGGHLT>45 and BhabhaVetoHLT==0 and ggVetoHLT==0',path,[1])

    # L1Emulation('','NcHLT>=2 and EC1HLT>0.5 and EC2HLT>0.5 and AngGGHLT>45',path,[200])

    L1Emulation('Hadron1', 'NtHLT>=3 and NltHLT>=2', path, [1])

    L1Emulation('Hadron2', 'NcHLT>=4 and AngGGCMSHLT>0.785 and AngGGCMSHLT<2.967 and eclBhabhaVetoHLT==0', path, [1])

    L1Emulation('Neutral', 'NcHLT>=3 and AngGGCMSHLT>0.349 and AngGGCMSHLT<2.967 and eclBhabhaVetoHLT==0', path, [1])

    L1EmulationPrint(path)


def L1Emulation_Veto(path):
    """
    This function adds the veto triggers to the path
    """
    # eclBhabhaVetoHLT
    L1Emulation('eclBhabhaVeto', 'EC1HLT>0 and EC2HLT>0.0 and EC12HLT>7.0 and AngGGCMSHLT>1.745', path, [1])

    # BhabhaVetoHLT, the logic "eclBhabhaVetoHLT" could is available here
    L1Emulation('BhabhaVeto', 'eclBhabhaVetoHLT==1 and NtHLT==2 and NtcHLT==2', path, [1])

    # Bhabha veto with single track
    L1Emulation(
        'SBhabhaVeto',
        'EC1HLT > 0 and EC2HLT > 0.0 and EC12HLT > 5.0 and AngGGCMSHLT > 1.745 and' +
        ' NtHLT == 1 and NtcHLT == 1 and ENeutralHLT > 0.5 and PT1HLT > 2.5 and ET1HLT > 2.0',
        path,
        [1])

    # ggVetoHLT
    L1Emulation('ggVeto', 'eclBhabhaVetoHLT==1 and NtHLT==0', path, [1])


def L1Emulation(trigger_name, selection_criteria, path, prescalefactor):
    """
    This function adds the L1Emulator module to a path
    """

    # load L1Emulation module
    L1Emulator = register_module('L1Emulation')
    L1Emulator.set_name('L1Emulator_' + trigger_name)
    L1Emulator.param('TRG', selection_criteria)
    L1Emulator.param('TRG_Scale', prescalefactor)
    path.add_module(L1Emulator)


def L1EmulationPrint(path):
    """
    This function adds the L1EmulatorPrint module to a path
    """
    L1Emulatorprint = register_module('L1EmulationPrint')
    path.add_module(L1Emulatorprint)
    L1Emulatorprint.logging.log_level = LogLevel.INFO
