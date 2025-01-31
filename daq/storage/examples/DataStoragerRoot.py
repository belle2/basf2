#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import hbasf2
import sys
import socket


b2.logging.add_udp(socket.gethostname(), 6900)

hltid = [int(i) for i in socket.gethostname().split('store') if i.isdigit()][0]
hltname = 'HLT%02d' % hltid
ndisk = 3
if hltid == 1 or hltid == 2:
    ndisk = 4

argv = sys.argv

b2.set_nprocesses(int(argv[1]))  # zmq

b2.conditions.override_globaltags(["online"])

# Create main path
main = b2.create_path()
outpath = b2.create_path()
phypath = b2.create_path()

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# set_debug_level(30)

zmq2ds = b2.register_module('StorageZMQ2Ds')
zmq2ds.param('addExpressRecoObjects', 1)
zmq2ds.param('inInitialize', True)
zmq2ds.param('input', argv[2])  # hbasf2

ds2zmq = b2.register_module('HLTDs2ZMQ')
ds2zmq.param('output', argv[3])  # hbasf2
ds2zmq.param('raw', False)

ds2zmq_phy = b2.register_module('HLTDs2ZMQ')
ds2zmq_phy.param('output', argv[4])  # hbasf2
ds2zmq_phy.param('raw', False)

output_root = b2.register_module('StorageRootOutput')
output_root.param('additionalDataDescription', {"dataLevel": "raw"})
output_root.param('compressionAlgorithm', int(argv[5]))
output_root.param('compressionLevel', int(argv[6]))
output_root.param('runType', argv[7])
output_root.param('HLTName', hltname)
output_root.param('nDisk', ndisk)
output_root.param('skipFirstEvent', True)
output_root.param('outputSplitSize', 210)  # Megabytes, should be controlled outside of basf2

hltskim = b2.register_module('TriggerSkim')
# This trigger line should be controlled outside of the script
# Maybe, through config file in daq_slc
hltskim.param('triggerLines',
              [
               ('software_trigger_cut&skim&accept_dstar_1', 1),
               ('software_trigger_cut&skim&accept_dstar_2', 2)
               ])
hltskim.param('resultOnMissing', 1)
hltskim.param('skipFirstEvent', True)
hltskim.if_value(">0", phypath, b2.AfterConditionPath.END)

output_root.set_property_flags(b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)  # zmq
zmq2ds.set_property_flags(b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)  # zmq
ds2zmq.set_property_flags(b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)  # zmq
ds2zmq_phy.set_property_flags(b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)  # zmq
hltskim.set_property_flags(b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)  # zmq


# Add modules to main path
main.add_module(zmq2ds)  # zmq

# if "null" not in argv[7].lower():
if "hlttest" in argv[7].lower():
    main.add_module(output_root)

main.add_module(ds2zmq)
main.add_module(hltskim)
phypath.add_module(ds2zmq_phy)

# Process all events
hbasf2.processNumbered(main, [], False, True)  # zmq
