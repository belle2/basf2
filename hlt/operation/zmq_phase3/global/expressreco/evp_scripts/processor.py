#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
import hbasf2
from softwaretrigger import constants
from softwaretrigger.processing import finalize_zmq_path, setup_basf2_and_db, start_zmq_path

args = setup_basf2_and_db(zmq=True)

path, reco_path = start_zmq_path(args, location=constants.Location.expressreco)

# Monitor/Debug modules
reco_path.add_module('MonitorData')
reco_path.add_module('ElapsedTime', EventInterval=10000)

finalize_zmq_path(path, args, location=constants.Location.expressreco)

basf2.print_path(path)
hbasf2.process(path, [args.dqm, args.output], True)
