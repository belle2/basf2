#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# RecvStream0.py : DAQ comoponent running on a readout PC.
# This component receive data from eb0 and send data to eb1tx.
#
######################################################

import basf2 as b2
import sys
argvs = sys.argv
if len(argvs) != 4:
    print('Usage : RecvStream1.py <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>')
    sys.exit()

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.INFO)

# Modules
receiver = b2.register_module('DeSerializerPC')
dump = b2.register_module('SeqRootOutput')
converter = b2.register_module('Convert2RawDet')

# Receiver
receiver.param('NodeID', 3)  # ROPC node ID (only used for Run control)
receiver.param('NumConn', 2)
receiver.param('HostNameFrom', ['localhost', 'localhost'])
receiver.param('PortFrom', [34008, 34009])
receiver.param('EventDataBufferWords', 4801)
receiver.param('MaxTime', -1.)
receiver.param('MaxEventNum', -1)
receiver.param('NodeName', argvs[3])  # node name for Run control
use_shm_flag = int(argvs[1])
receiver.param('UseShmFlag', use_shm_flag)

# Dump
dump.param('outputFileName', 'root_output.sroot')

# Sender
sender = b2.register_module('Serializer')
sender.param('DestPort', 36000)
sender.param('LocalHostName', 'localhost')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(receiver)
main.add_module(converter)
main.add_module(dump)
# main.add_module(sender)

# Process all events
b2.process(main)
