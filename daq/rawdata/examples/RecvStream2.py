#!/usr/bin/env python
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
    print 'Usage : RecvStream2.py <COPPER hostname> <Use NSM(Network Shared Memory)? yes=1/no=0> <port # of eb0> <NSM nodename>'
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# Modules
processor = register_module('DesSerPrePCMain')

# Processor
processor.param('HostNameFrom', argvs[1])
processor.param('PortTo', int(argvs[3]))

processor.param('NodeID', 3)  # ROPC node ID (only used for Run control)
processor.param('NodeName', argvs[4])  # node name for Run control
use_shm_flag = int(argvs[2])
processor.param('UseShmFlag', use_shm_flag)

# Create main path
main = create_path()

# Add modules to main path
main.add_module(processor)

# Process all events
process(main)
