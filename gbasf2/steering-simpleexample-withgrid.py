#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
swver = 'build_2010-12-13'
# (optional) Files to be send with the job
inputsandboxfiles = 'file1.txt,file2.txt'
# (optional) max events - the maximum number of events to use
maxevents = '100000'

############################
# Basf2 configuration     #
############################
# register modules
evtmetagen = fw.register_module('EvtMetaGen')
evtmetainfo = fw.register_module('EvtMetaInfo')

# set parameters in modules
param_evtmetagen = {
    'ExpList': [1],
    'RunList': [1],
    'EvtStartList': [0],
    'EvtEndList': [100],
    }

evtmetagen.param(param_evtmetagen)

# create path and add modules
main = fw.create_path()
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

# process
fw.process(main)
