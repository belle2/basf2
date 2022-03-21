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
# Steering file to create charged/mixed MC cdst files
# needed for the DecayFile/Multiplicity validation
#############################################################

import basf2 as b2
import generators as ge
import simulation as si
import L1trigger as l1
import reconstruction as re

# set MC type (charged or mixed) and output path
MCtype = 'charged'
outputpath = MCtype+'R4.cdst.root'

# create path
main = b2.create_path()

# default to early phase 3 (exp=1003), run 0, number of events = 2000
main.add_module("EventInfoSetter", expList=1003, runList=0, evtNumList=2000)

# generate BBbar events
ge.add_evtgen_generator(path=main, finalstate=MCtype)

# detector simulation
si.add_simulation(path=main)

# trigger simulation
l1.add_tsim(path=main)

# reconstruction
re.add_reconstruction(path=main, reconstruct_cdst='rawFormat')

# finally add cdst output
main.add_module('RootOutput', outputFileName=outputpath)

# process events and print call statistics
b2.process(path=main)
print(b2.statistics)
