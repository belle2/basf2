#!/usr/bin/env python
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
if len(argvs) != 3:
    print 'Usage : RecvStream1.py <Use shared memory? yes=1/no=0> <port # of eb0>'
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# Modules
receiver = register_module('DeSerializerPC')
# receiver = register_module('DeSerializerHLT')
# dump = register_module('PrintCollections')
dump = register_module('SeqRootOutput')
perf = register_module('DAQPerf')
output = register_module('PrintData')

# Receiver
receiver.param('NodeID', 3)
receiver.param('NumConn', 1)
receiver.param('HostNameFrom', ['localhost', 'cpr007'])
# receiver.param('HostNameFrom', ['ttd3', 'cpr006'])
# receiver.param('HostNameFrom', ['cpr006', 'ttd3'])
receiver.param('PortFrom', [int(argvs[2]), 36000])
# receiver.param('PortFrom', [33000, 33000])

receiver.param('EventDataBufferWords', 4801)
receiver.param('MaxTime', 20.)
# receiver.param('MaxTime', -1.)
# receiver.param('MaxEventNum', 400000)
receiver.param('MaxEventNum', -1)
use_shm_flag = int(argvs[1])
receiver.param('UseShmFlag', use_shm_flag)
# receiver.param('DumpFileName', 'cpr006_dump.dat')

# Histo Module
# histo = register_module('HistoManager')
# main.add_module (histo)
histo = register_module('DqmHistoManager')
histo.param('HostName', 'ropc01')
histo.param('Port', 9991)
histo.param('DumpInterval', 10)
histo.param('HistoFileName', 'ropc_histofile.root')

# Monitor module
monitor = register_module('MonitorDataCOPPER')

# Dump
# dump.param('outputFileName', 'root_output.root')
dump.param('outputFileName', '~yamadas/root_output.root')
dump.param('compressionLevel', 0)
# Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticeable effect.

# Perf
perf.param('Cycle', 100000)
perf.param('MonitorSize', True)

# Create main path
main = create_path()

# Add modules to main path
main.add_module(receiver)

# if use_shm_flag != 0:
#    main.add_module(histo)
#    main.add_module(monitor)
main.add_module(histo)
main.add_module(monitor)
main.add_module(dump)
# main.add_module(output)
# main.add_module(perf)

# Process all events
process(main)
