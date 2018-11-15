#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to weight tracks using
# LooKUpTable from the database
#
# Contributors: I. Komarov (April 2018)
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variableCollections as vc
import variableCollectionsTools as vct
import stdCharged as stdc
import variables as va

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# use standard final state particle lists
# creates "pi+:all" ParticleList (and c.c.)
ma.fillParticleListFromMC(decayString='pi+:gen', cut='', path=my_path)

# ID of weight table is taked from B2A904
weight_table_id = "ParticleReweighting:TestMomentum"

# We know what weight info will be added (see B2A904),
# so we add aliases and add it ot tools
va.variables.addAlias('Weight', 'extraInfo(' + weight_table_id + '_Weight)')
va.variables.addAlias('StatErr', 'extraInfo(' + weight_table_id + '_StatErr)')
va.variables.addAlias('SystErr', 'extraInfo(' + weight_table_id + '_SystErr)')
va.variables.addAlias('binID', 'extraInfo(' + weight_table_id + '_binID)')


# We configure weighing module
reweighter = register_module('ParticleWeighting')
reweighter.param('tableName', weight_table_id)
reweighter.param('particleList', 'pi+:gen')
my_path.add_module(reweighter)


pivars = ['p', 'pz', 'Weight', 'StatErr', 'SystErr', 'binID']

# Saving variables to ntuple
output_file = 'B2A905-ApplyWeightsToTracks.root'
ma.variablesToNtuple(decayString='pi+:gen',
                     variables=pivars,
                     treename='pion',
                     filename=output_file,
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
