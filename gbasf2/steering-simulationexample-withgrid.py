#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

############################
# gBasf2 configuration     #
############################
# Name for project
project = 'e1-testdata'
# (optional) Job priority
priority = '1'
# Experiments (comma separated list)
experiments = '1'
# Metadata query
query = 'runL > 0'
# Type of Data ('data' or 'MC')
type = 'data'
# estimated Average Events per Minute (eg official MC = 40)
evtpermin = '40'
# Software Version
swver = 'build-2010-12-13'
# (optional) Files to be send with the job
inputsandboxfiles = 'file1.txt,file2.txt'
# (optional) max events - the maximum number of events to use
maxevents = '100000'

############################
# Basf2 configuration     #
############################
# Register modules
evtmetagen = fw.register_module('EvtMetaGen')
evtmetainfo = fw.register_module('EvtMetaInfo')
paramloader = fw.register_module('ParamLoaderXML')
geobuilder = fw.register_module('GeoBuilder')
g4sim = fw.register_module('SimModule')
cdcdigitizer = fw.register_module('CDCDigitizer')
out = fw.register_module('SimpleOutput')

##Set parameters
param_evtmetagen = {
    'ExpList': [1],
    'RunList': [2],
    'EvtStartList': [7001],
    'EvtEndList': [9000],
    }

evtmetagen.param(param_evtmetagen)

paramloader.param('InputFileXML', os.path.join(basf2datadir,
                  'simulation/Belle2.xml'))

param_g4sim = {'RandomSeed': 5486, 'MacroName': 'g4sim.macro'}
g4sim.param(param_g4sim)

out.param('outputFileName', 'sample_file_3.root')

##Create paths
main = fw.create_path()

##Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(g4sim)
main.add_module(cdcdigitizer)
main.add_module(out)

##Process events
fw.process(main)
