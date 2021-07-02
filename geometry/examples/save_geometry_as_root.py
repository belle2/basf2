#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file creates the Belle II detector
# geometry, and saves it as a Root file.
#
# This currently doesn't work for the ECL as the ECL uses custom volume types
# which cannot be converted to ROOT so for this to work we need to create a
# geometry without the ECL.
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import Path, process

main = Path()
# EventInfoSetter - generate event meta data
main.add_module("EventInfoSetter", expList=[0], runList=[0], evtNumList=[1])
# Geometry parameter loader
main.add_module('Gearbox')
# Geometry builder, ignore database and exclude the ECL as we cannot exclude
# things when loading Geometry from the database. Alternatively one could
# create a database geometry configuration without the ecl.
main.add_module('Geometry', useDB=False, excludedComponents=['ECL', 'Cryostat'])
# Saves the geometry as a Root file
main.add_module('ExportGeometry', Filename='Belle2Geo.root')
# Process one event
process(main)
