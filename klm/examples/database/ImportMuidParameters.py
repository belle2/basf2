#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################
# simple steering file to execute MuidParameterDBReaderWriter
# writeMuidParameters read parameters from .xml file and
# create dbobject ./localdb/*MuidParameters*.root.
# readMuidParameters read dbobject MuidParameters from database
###############################################################

import basf2
import ROOT
from ROOT.Belle2 import MuidParameterDBReaderWriter

basf2.set_log_level(basf2.LogLevel.INFO)

eventinfo = basf2.register_module('EventInfoSetter')
eventinfo.param({'evtNumList': [0], 'runList': [0]})
paramloader = basf2.register_module('Gearbox')

main = basf2.create_path()
main.add_module(eventinfo)
main.add_module(paramloader)
basf2.process(main)

# run the MuidParameters writer, create dbobject payload in ./localdb
dbWriter = MuidParameterDBReaderWriter()
dbWriter.writeMuidParameters()

# run the MuidParameters reader
# dbReader = MuidParameterDBReaderWriter()
# dbReader.readMuidParameters()
