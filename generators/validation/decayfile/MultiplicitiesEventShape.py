#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# Steering file to reconstruct charged/mixed MC cdst files.
# Used to validate changes between decay files.
# Prints multiplicities of generated particles.
# Uses the SplitMultiplicities module to list multiplicities
# of Kaons depending on B meson flavour in a separate tree.
# In addition gives tree containing generated event shapes.
#############################################################

"""
<header>
    <input>../charged.cdst.root</input>
    <output>MCvalidation.root</output>
    <description>Determining multiplicities of different particles on generator level</description>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
from variables import variables as v
from variables import collections as c
from SplitMultiplicities import SplitMultiplicities

path = b2.create_path()


def define_ups_aliases():
    '''Define aliases to write out multiplicities'''
    alias_dict = {}
    alias_dict['nPIp'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(211))'
    alias_dict['nPI0'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(111))'
    alias_dict['nETA'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(221))'
    alias_dict['nETAprim'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(331))'
    alias_dict['nPHI'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(333))'
    alias_dict['nRHOp'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(213))'
    alias_dict['nRHO0'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(113))'
    alias_dict['nKp'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(321))'
    alias_dict['nKL0'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(130))'
    alias_dict['nKS0'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(310))'
    alias_dict['nKstar0'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(313))'
    alias_dict['nKstarp'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(323))'
    alias_dict['nDp'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(411))'
    alias_dict['nD0'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(421))'
    alias_dict['nBp'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(521))'
    alias_dict['nB0'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(511))'
    alias_dict['nJPSI'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(443))'
    alias_dict['nELECTRON'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(11))'
    alias_dict['nENEUTRINO'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(12))'
    alias_dict['nMUON'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(13))'
    alias_dict['nMNEUTRINO'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(14))'
    alias_dict['nTAUON'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(15))'
    alias_dict['nTNEUTRINO'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(16))'
    alias_dict['nPHOTON'] = 'genUpsilon4S(NumberOfMCParticlesInEvent(22))'
    alias_dict['GenUp4S_nTracks'] = 'genUpsilon4S(nTracks)'
    return(alias_dict)


def define_split_aliases():
    '''Define aliases for extraInfo resulting from SplitMultiplicities module'''
    alias_dict = {}
    alias_dict['gen_Kp'] = 'extraInfo(nGen_321)'
    alias_dict['gen_Kp_Bp'] = 'extraInfo(nGen_321_Bp)'
    alias_dict['gen_Kp_Bm'] = 'extraInfo(nGen_321_Bm)'
    alias_dict['gen_Kp_B0'] = 'extraInfo(nGen_321_B0)'
    alias_dict['gen_Kp_antiB0'] = 'extraInfo(nGen_321_antiB0)'
    alias_dict['gen_Km'] = 'extraInfo(nGen_-321)'
    alias_dict['gen_Km_Bp'] = 'extraInfo(nGen_-321_Bp)'
    alias_dict['gen_Km_Bm'] = 'extraInfo(nGen_-321_Bm)'
    alias_dict['gen_Km_B0'] = 'extraInfo(nGen_-321_B0)'
    alias_dict['gen_Km_antiB0'] = 'extraInfo(nGen_-321_antiB0)'
    alias_dict['gen_K0'] = 'extraInfo(nGen_311)'
    alias_dict['gen_K0_Bp'] = 'extraInfo(nGen_311_Bp)'
    alias_dict['gen_K0_Bm'] = 'extraInfo(nGen_311_Bm)'
    alias_dict['gen_K0_B0'] = 'extraInfo(nGen_311_B0)'
    alias_dict['gen_K0_antiB0'] = 'extraInfo(nGen_311_antiB0)'
    alias_dict['gen_antiK0'] = 'extraInfo(nGen_-311)'
    alias_dict['gen_antiK0_Bp'] = 'extraInfo(nGen_-311_Bp)'
    alias_dict['gen_antiK0_Bm'] = 'extraInfo(nGen_-311_Bm)'
    alias_dict['gen_antiK0_B0'] = 'extraInfo(nGen_-311_B0)'
    alias_dict['gen_antiK0_antiB0'] = 'extraInfo(nGen_-311_antiB0)'
    return(alias_dict)


def add_aliases(alias_dict={}):
    '''Here we add the aliases defined in define_{split/ups}_aliases'''
    for key, value in alias_dict.items():
        v.addAlias(key, value)


# read input file
ma.inputMdstList('../charged.cdst.root', path)

# pick one charged track per event needed for the SplitMultiplicities module
ma.fillParticleList('pi+:sel', '',  path=path)
ma.rankByHighest('pi+:sel', variable="random", numBest=1, outputVariable="Rank", path=path)

path.add_module(SplitMultiplicities('pi+:sel', 321))  # K+
path.add_module(SplitMultiplicities('pi+:sel', -321))  # K-
path.add_module(SplitMultiplicities('pi+:sel', 311))  # K0
path.add_module(SplitMultiplicities('pi+:sel', -311))  # anti_K0

# fill generated Upsilon particle list
ma.fillParticleListFromMC("Upsilon(4S):MC", '', True, path=path)

# fill other generated particle lists needed for event shape
pions = ("pi+:MC", '')
kaons = ("K+:MC", '')
muons = ("mu+:MC", '')
electrons = ("e+:MC", '')
protons = ("p+:MC", '')
photons = ("gamma:MC", '')
klongs = ("K_L0:MC", '')
ma.fillParticleListsFromMC([pions, kaons, muons, electrons, protons, klongs, photons], path=path)

# build event shape
ma.buildEventShape(['pi+:MC', "K+:MC", "mu+:MC", "e+:MC", "p+:MC", "K_L0:MC", "gamma:MC"], path=path)

# create a dictionary of multiplicity variable aliases
Multi_aliasDict = define_ups_aliases()
Split_aliasDict = define_split_aliases()

# add the multiplicity aliases into variables
add_aliases(Multi_aliasDict)
add_aliases(Split_aliasDict)
multi_variables = list(Multi_aliasDict.keys())
split_variables = list(Split_aliasDict.keys())

# add event shape variables to a list
eventshape_variables = c.event_shape

# write out the trees containing the multiplicities, split multiplicities for kaons and the event shape variables
ma.variablesToNtuple(
    'Upsilon(4S):MC',
    treename="Multiplicities",
    variables=multi_variables,
    filename='MCvalidation.root',
    path=path)
ma.variablesToNtuple('pi+:sel', treename="Split", variables=split_variables, filename='MCvalidation.root', path=path)
ma.variablesToNtuple(
    'Upsilon(4S):MC',
    treename="EventShape",
    variables=eventshape_variables,
    filename='MCvalidation.root',
    path=path)

progress = ma.register_module('Progress')
path.add_module(progress)
b2.process(path)
print(b2.statistics)
