#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################
# simple steering file to execute MuidParameterDBReaderWriter
# writeMuidParameters read parameters from .xml file and
# create dbobject ./localdb/*MuidParameters*.root.
# readMuidParameters read dbobject MuidParameters from database
###############################################################

from basf2 import *
import ROOT
from ROOT.Belle2 import MuidParameterDBReaderWriter
import os
import sys
import glob
import subprocess

set_log_level(LogLevel.INFO)
# use local database
reset_database()
use_local_database("localdb/database.txt", "localdb")

eventinfo = register_module('EventInfoSetter')
eventinfo.param({'evtNumList': [1], 'runList': [1]})
# read muid parameters from xml file needs Gearbox
paramloader = register_module('Gearbox')
main = create_path()
main.add_module(eventinfo)
main.add_module(paramloader)
process(main)

# run the MuidParameters writer, create dbobject payload in ./localdb
# dbWriter = MuidParameterDBReaderWriter()
# dbWriter.writeMuidParameters()

# run the MuidParameters reader
dbReader = MuidParameterDBReaderWriter()
dbReader.readMuidParameters()
