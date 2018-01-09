#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to run the framework
# for different experiment, run and event numbers.
#
# In the example below, basf2 will run on and display
# the following experiment, run and event numbers:
#
# Experiment 71, Run  3, 4 Events
# Experiment 71, Run  4, 6 Events
# Experiment 73, Run 10, 2 Events
# Experiment 73, Run 20, 5 Events
# Experiment 73, Run 30, 3 Events
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *
import sys
argvs = sys.argv

if len(argvs) < 6:
    print('Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER nodeID> <bit flag of FINNESEs> '
          '<Use shared memory? yes=1/no=0> <nodename>')
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Reader
# reader = register_module('HSLBReaderArray')
reader = register_module('DeSerializerCOPPER')
reader.param('NodeID', int(argvs[2]))
reader.param('FinesseBitFlag', int(argvs[3]))
use_shm_flag = int(argvs[4])
reader.param('UseShmFlag', use_shm_flag)
reader.param('NodeName', argvs[5])
# reader.param('DumpFileName', 'COPPERdump.dat' )

# Histo Module
# histo = register_module('HistoManager')
# main.add_module (histo)
histo = register_module('DqmHistoManager')
histo.param('HostName', 'ropc04')
histo.param('Port', 9991)
histo.param('DumpInterval', 10)

histo.param('HistoFileName', 'histo_file_'.join([argvs[1], '.root']))

# Monitor module
monitor = register_module('MonitorDataCOPPER')

# Sender
sender = register_module('Serializer')
sender.param('DestPort', 30000)
# sender.param('LocalHostName', 'cpr006')
sender.param('LocalHostName', argvs[1])

# Create main path
main = create_path()

# Add modules to main path
main.add_module(reader)
# if use_shm_flag != 0:
#    main.add_module(histo)
#    main.add_module(monitor)
# main.add_module(sender)

# Process all events
process(main)
