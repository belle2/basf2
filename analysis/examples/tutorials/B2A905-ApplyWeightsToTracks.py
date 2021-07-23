#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to weight tracks using                  #
# LooKUpTable from the database                                          #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables as va
import os

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# use standard final state particle lists
# creates "pi+:all" ParticleList (and c.c.)
ma.fillParticleListFromMC(decayString='pi+:gen', cut='', path=my_path)

# ID of weight table is taken from B2A904
weight_table_id = "ParticleReweighting:TestMomentum"

if not os.getenv('BELLE2_EXAMPLES_DATA_DIR'):
    b2.B2FATAL("You need the example data installed. Run `b2install-data example` in terminal for it.")

db_location = os.getenv('BELLE2_EXAMPLES_DATA_DIR') + '/database/'
b2.conditions.prepend_testing_payloads(db_location + 'database.txt')

# We know what weight info will be added (see B2A904),
# so we add aliases and add it to the tools
va.variables.addAlias('Weight', 'extraInfo(' + weight_table_id + '_Weight)')
va.variables.addAlias('StatErr', 'extraInfo(' + weight_table_id + '_StatErr)')
va.variables.addAlias('SystErr', 'extraInfo(' + weight_table_id + '_SystErr)')
va.variables.addAlias('binID', 'extraInfo(' + weight_table_id + '_binID)')


# We configure weighing module
reweighter = b2.register_module('ParticleWeighting')
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
