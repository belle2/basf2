#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
#
# RecvStream0.py : DAQ comoponent running on a readout PC.
# This component receive data from eb0 and send data to eb1tx.
#
######################################################

from basf2 import *
import sys
argvs = sys.argv
if len(argvs) != 4:
    print 'Usage : RecvStream1.py <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>'
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# Modules
receiver = register_module('DeSerializerPC')
dump = register_module('RootOutput')
perf = register_module('DAQPerf')
output = register_module('PrintData')

# Receiver
receiver.param('NodeID', 3)  # ROPC node ID (only used for NSM)
receiver.param('NumConn', 1)
receiver.param('HostNameFrom', ['localhost', 'cpr007'])
receiver.param('PortFrom', [int(argvs[2]), int(argvs[2])])
receiver.param('EventDataBufferWords', 4801)
receiver.param('MaxTime', -1.)
receiver.param('MaxEventNum', -1)
receiver.param('NodeName', argvs[3])  # node name for NSM
use_shm_flag = int(argvs[1])
receiver.param('UseShmFlag', use_shm_flag)


# Monitor module
monitor = register_module('MonitorDataCOPPER')

# Dump
dump.param('outputFileName', 'root_output.root')

# Sender
sender = register_module('Serializer')
sender.param('DestPort', 36000)
sender.param('LocalHostName', 'localhost')

# Perf
perf.param('Cycle', 100000)
perf.param('MonitorSize', True)

# Create main path
main = create_path()

# Add modules to main path
main.add_module(receiver)
# main.add_module(monitor)
# main.add_module(dump)
main.add_module(sender)
# main.add_module(output)
# main.add_module(perf)

# Process all events
process(main)
