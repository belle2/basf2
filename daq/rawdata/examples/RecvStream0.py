#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# RecvStream0.py : DAQ comoponent running on a readout PC.
# This component receive data from COPPER and send data to eb0 process.
#
######################################################

from basf2 import *
import sys
argvs = sys.argv
if len(argvs) != 5:
    print('Usage : RecvStream0.py <COPPER hostname> <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>')
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# Modules
receiver = register_module('DeSerializerPrePC')
dump = register_module('RootOutput')
output = register_module('PrintData')

# Receiver
receiver.param('NodeID', 3)  # ROPC node ID (only used for Run control)
receiver.param('NumConn', 1)
receiver.param('HostNameFrom', [argvs[1], 'aiueohost'])
receiver.param('PortFrom', [30000, 30000])
receiver.param('EventDataBufferWords', 4801)
receiver.param('MaxTime', -1.)
receiver.param('MaxEventNum', -1)
receiver.param('NodeName', argvs[4])  # node name for Run control
use_shm_flag = int(argvs[2])
receiver.param('UseShmFlag', use_shm_flag)

# Dump
dump.param('outputFileName', 'root_output.root')

# Sender
sender = register_module('Serializer')
sender.param('DestPort', int(argvs[3]))
sender.param('LocalHostName', 'localhost')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(receiver)
# main.add_module(dump)
main.add_module(sender)

# Process all events
process(main)
