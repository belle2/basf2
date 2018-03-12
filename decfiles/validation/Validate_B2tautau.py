#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''Generates a small sample of B to tau+ tau- for validation purposes'''
__author__ = 'Sam Cunliffe'

import modularAnalysis as ma
from generators import add_evtgen_generator
from ROOT import Belle2 as b2

# generate events
ma.setupEventInfo(10000)
add_evtgen_generator(analysis_main, 'signal', Belle2.FileSystem.findFile('decfiles/dec/1120600000.dec'))
ma.loadGearbox()

# grab all taus, and also find the signal
ma.fillParticleListsFromMC([('tau+:gen', '')])
ma.findMCDecay('B0:tt', 'B0 -> tau+:gen tau-:gen')

# declare branchs and format for output root file
tools = ['EventMetaData', '^B0']
tools += ['Kinematics', '^B0 -> ^tau+ ^tau-']
ma.ntupleFile('Validate_B2tautau.root')
ma.ntupleTree('BtoTauTau', 'B0:tt', tools)

# look for all other B decays for statistics purposes
ma.findMCDecay('B+', 'B+')
ma.findMCDecay('B-', 'B-')
ma.findMCDecay('B0', 'B0')
ma.findMCDecay('B_s0', 'B_s0')
ma.findMCDecay('anti-B0', 'anti-B0')
ma.findMCDecay('anti-B_s0', 'anti-B_s0')

# print a summary
ma.summaryOfLists(['B0:tt', 'tau+:gen', 'B+', 'B0', 'anti-B0', 'B-'])

# run all modules in the path
ma.process(ma.analysis_main)
print(ma.statistics)
