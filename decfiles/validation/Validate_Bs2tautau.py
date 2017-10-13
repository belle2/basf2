#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''Generates a small sample of Bs to tau+ tau- for validation purposes'''
__author__ = 'Sam Cunliffe'

import modularAnalysis as ma
from ROOT import Belle2 as b2
from beamparameters import add_beamparameters

# set the BeamParameters for running at Y(5S)
beamparameters = add_beamparameters(ma.analysis_main, "Y5S")
ma.print_params(beamparameters)

# specify number of events to be generated
ma.analysis_main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=10000)  # fast because no detector sim

# generate BBbar events
evtgeninput = ma.register_module('EvtGenInput')
evtgeninput.param('ParentParticle', 'Upsilon(5S)')
evtgeninput.param('userDECFile', b2.FileSystem.findFile('../dec/1320600040.dec'))
ma.analysis_main.add_module(evtgeninput)


# grab all taus, and also find the signal
ma.fillParticleListsFromMC([('tau+:gen', '')])
ma.findMCDecay('B_s0:tt', 'B_s0 -> tau+:gen tau-:gen')

# declare branchs and format for output root file
tools = ['EventMetaData', '^B0']
tools += ['Kinematics', '^B_s0 -> ^tau+ ^tau-']
ma.ntupleFile('Validate_Bs2tautau.root')
ma.ntupleTree('BstoTauTau', 'B_s0:tt', tools)

# look for all other B decays for statistics purposes
ma.findMCDecay('B+', 'B+')
ma.findMCDecay('B-', 'B-')
ma.findMCDecay('B0', 'B0')
ma.findMCDecay('B_s0', 'B_s0')
ma.findMCDecay('anti-B0', 'anti-B0')
ma.findMCDecay('anti-B_s0', 'anti-B_s0')

# print a summary
ma.summaryOfLists(['B_s0:tt', 'tau+:gen', 'B+', 'B0', 'anti-B0', 'B-'])

# run all modules in the path
ma.process(ma.analysis_main)
print(ma.statistics)
