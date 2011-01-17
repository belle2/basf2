#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

############################
# gBasf2 configuration     #
############################
# Name for project
project = 'e13-e57-test'
# (optional) Job priority
priority = '1'
# Experiments (comma separated list)
experiments = '13,57'
# Metadata query
query = 'runL > 10 and runH < 12'
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

# Create module
test = fw.register_module('Hello')

# Create path
main = fw.create_path()

# Add module to path
main.add_module(test)

# Start event processing
fw.process(main, 100)
