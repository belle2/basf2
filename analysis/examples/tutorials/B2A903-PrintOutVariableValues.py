#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to print out the
# contents of the StoreArray<MCParticle>.
#
# This is useful for debugging purposes, e.g. to check if
# the MC files contain the desired processes/decays.
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (September 2018)
#
######################################################

import basf2 as b2
import modularAnalysis as ma

#  check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/B2pi0D_D2hh_D2hhh_B2munu.root'):
    b2.B2FATAL("You need the example data installed. Run `b2mount-tutorial-cloud` in terminal for it.")

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/B2pi0D_D2hh_D2hhh_B2munu.root',
             path=my_path)

# create charged kaon and pion particle lists with names "K-:gen" and "pi-:gen"
# all generated kaons(pions) will be added to the kaon(pion) list
ma.fillParticleListFromMC(decayString='K-:gen', cut='', path=my_path)
ma.fillParticleListFromMC(decayString='pi-:gen', cut='', path=my_path)

# print charge, energy and total momentum of generated kaons
# and x,y,z components of momenta for generated pions
#
# the list of all available variables can be obtained by executing
# basf2 analysis/scripts/variables.py
printVariableValues(list_name='K-:gen', var_names=['charge', 'E', 'p'], path=my_path)
printVariableValues(list_name='pi-:gen', var_names=['charge', 'px', 'py', 'pz'], path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
