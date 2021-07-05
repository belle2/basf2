##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This file includes helpers used in the local example script and are not needed for the production environment
import basf2
from time import sleep
import zmq


class SleepOnInitModule(basf2.Module):
    """Helper module to sleep 20 seconds on init (to mimik geometry loading)"""

    def __init__(self):
        """Make the module parallel processing certified"""
        super().__init__()
        self.set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)

    def initialize(self):
        """Sleep 20 seconds"""
        sleep(20)


def add_input_module(path, input_address, add_expressreco_objects):
    """Add the ZMQ input module in the settings used for the examples"""
    input_module = path.add_module("HLTZMQ2Ds", input=input_address, addExpressRecoObjects=add_expressreco_objects)

    return input_module


def add_reco_modules(path, dqm_address, mimik_startup):
    """Add the reconstruction modules in the settings used for the examples"""
    if mimik_startup:
        path.add_module(SleepOnInitModule())

    if dqm_address:
        path.add_module("HLTDQM2ZMQ", output=dqm_address, sendOutInterval=5)
        path.add_module("DAQMonitor")


def add_output_module(path, output_address, raw):
    """Add the ZMQ output module in the settings used for the examples"""
    path.add_module("HLTDs2ZMQ", output=output_address, raw=raw)


def create_socket(context, address):
    """Create a socket out of the given address"""
    socket = context.socket(zmq.DEALER)
    socket.connect(address)

    return socket


def get_sockets(settings):
    """Create all monitoring sockets for the given settings dictionary"""
    ctx = zmq.Context()
    sockets = {name: create_socket(ctx, address) for name, address in settings["monitor"].items()}
    return sockets
