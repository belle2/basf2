#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import fillParticleList
import os
import sys

analysis_main = create_path()
emptypath = create_path()


def add_HLT_Y4S(path, filter=False):
    """
    This function is to do physics trigger, and add them to path
    @filter 1: open the filter of hlt, 0: close the filter of hlt
    @path add this module to the path
    """
    # create charged tracks list
    fillParticleList('pi+:HLT', '', False, path)

    # create gamma list
    fillParticleList('gamma:HLT', '', False, path)

    # hadron
    PTrigger('Hadron', 'NtHLT>=3 and VisibleEnergyHLT>2.5', path, 1)
    # Bhabha Accept1
    PTrigger(
        'BhabhaAccept1',
        'NtHLT>=2 and NtcHLT>=1 and PT1CMSHLT>=3.0 and PT12CMSHLT>4.0 and ' +
        'EC1CMSHLT>2.0 and maxAngTTCMSHLT>2.793',
        path, 4)

    # Bhabha Accept2
    PTrigger('BhabhaAccept2', 'NtHLT>=1 and PT1CMSHLT>=3.0 and AngGTHLT>2.618 and ENeutralHLT>2.0 ', path, 4)

    # HLT tau
    PTrigger('Tautau', 'NtHLT>=2 and PT1CMSHLT<5.0 and EtotHLT<9.0 and VisibleEnergyHLT<9.0', path, 2)

    # HLT 2Trk
    PTrigger('2Trk', 'NtHLT>=2 and NtHLT<=5 and PT1CMSHLT<5.0 and PT12CMSHLT<9.0 ', path, 3)

    # HLT 1Trk1cluster
    PTrigger('1Trk1Cluster', 'NtHLT>=1 and NtHLT<=2 and ENeutralHLT>1.0 and AngGTHLT>0.785 and NEidHLT==0', path, 3)

    # HLT Dimu1
    PTrigger('Mumu1', 'NtHLT>=2 and PT12CMSHLT>4.0 and PT1CMSHLT>3.0 and EC12CMSHLT<2', path, 4)

    # HLT Dimu2
    PTrigger('Mumu2', 'NtHLT==1 and PT1CMSHLT>3.0 and NEidHLT==0', path, 4)

    # HLT GG
    PTrigger('GG', 'NtHLT<=1 and NEidHLT==0 and EC12CMSHLT>4 and EC1CMSHLT>2', path, 5)

    # Fill HLTTag
    FillHLTTag(path, filter)


def PTrigger(triggername, selection_criteria, path, category):
    """
    This function adds the physics trigger module to a path
    @param selection_criteria the selection criteria of physics trigger
    @path module is added to this path
    @trigger_item the index of trigger items, start from 1
    @category the catregory that the trigger result will be stored
     in HLTTag, 1: Hadron, 2: Tautau, 3: LowMulti, 4: Bhabha, 5: Dimu, 6: GG, 7: other

    """
    # load L1Emulation module
    ptrigger = register_module('PhysicsTrigger')
    ptrigger.set_name("PhysicsTrigger_" + triggername)
    ptrigger.param('PT', selection_criteria)
    ptrigger.param('TriggerCategory', category)
    path.add_module(ptrigger)


def FillHLTTag(path, filter):
    """
    """
    fhlt = register_module('FillHLTTag')
    path.add_module(fhlt)
#    if filter :
#    fhlt.if_value('<1',emptypath)
