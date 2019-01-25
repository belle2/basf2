#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file creates the Belle II detector
# geometry, and saves it as a Root file.
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import Path, process

main = Path()
# EventInfoSetter - generate event meta data
main.add_module("EventInfoSetter", expList=[0], runList=[0], evtNumList=[1])
# Geometry parameter loader
main.add_module('Gearbox')
# Geometry builder
main.add_module('Geometry')
# Saves the geometry as a Root file
main.add_module('ExportGeometry', Filename='Belle2Geo.root')
# Process one event
process(main)
