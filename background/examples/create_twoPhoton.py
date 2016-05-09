#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Script to generate and simulate two photon events for PXD and SVD

Usage:
  basf2 [basf2_arguments...] create_twoPhoton.py -- realtime [run] [--pxd_only] [--luminosity=8e36]

notice the -- after the script name to make sure the remaining arguments are
passed to the script and not interpreted by basf2

Examples:
   basf2 create_twoPhoton.py -- --help

   show help about script arguments

   basf2 -p8 create_twoPhoton.py -- 2 7 --pxd_only

   create 2 milliseconds of pxd only background with run number in the file
   name set to 7 in parallel mode with 8 cores, output will be
   twoPhoton_2msPXD-0007.root

   basf2 create_twoPhoton.py -- 0.01 125 --luminosity=2e34

   create 0.01 millisecond of background for SVD and PXD with run number of
   125 with the target luminosity set to 2e34, output will be
   twoPhoton_0.01ms-0125.root
"""

import sys
import argparse
from basf2 import *
from beamparameters import add_beamparameters
# root interferes with command line options otherwise
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = 1
import ROOT
from ROOT import Belle2
# be less verbose
set_log_level(LogLevel.WARNING)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--pxd_only", action="store_true", default=False,
                    help="If given, output will only be for PXD, otherwise PXD and SVD")
parser.add_argument("realtime", type=float, default=1,
                    help="number of milli seconds to produce (default: %(default)s)")
parser.add_argument("run", nargs="?", type=int, default=0,
                    help="run number to be appended to the output file (default: %(default)s)")
parser.add_argument("--luminosity", type=float, default=8e35, metavar="LUMI",
                    help="design luminosity in 1/cm^2/s (default: %(default)s)")
arguments = parser.parse_args()

# realtime to simulate in ns
realtime = arguments.realtime*1e6

# pxd only or all?
pxd = arguments.pxd_only and "PXD" or ""
pxdfn = arguments.pxd_only and "PXD" or "usual"

# background type
bkgtype = 'twoPhoton'

# cross section of (uncut) QED events
crosssection = 7.28e+06  # +-1.48e+04 nb (nb = 10e-33 cm^2) from previous aafh run

# calculate number of events with given cross section
n_events = int(crosssection * 1e-33 * arguments.luminosity * realtime * 1e-9)

# output filename
output_filename = '%s_%gms-%s-%04d.root' % (bkgtype, realtime / 1e6, pxdfn, arguments.run)
output_filename = 'output/' + output_filename

# branches to save
branches = ["PXDSimHits"]
if not arguments.pxd_only:
    branches += ["SVDSimHits"]

print("Generating %d Events and saving it to %s" % (n_events, output_filename))

# fine tune the weights to optimize event generation. According to the
# documentation this should only affect the efficiency, not the result
aafh_params = {
    "mode": 5,  # ee -> eeee
    "rejection": 2,
    "maxSubgeneratorWeight": 2.5,
    "maxFinalWeight": 5,
    "subgeneratorWeights": [1.000e+00, 3.174e+02, 1.601e+10, 3.230e+10,
                            1.000e+00, 9.921e-01, 2.564e+00, 5.443e+00],
}


class DiscardEmptyVXD(Module):
    """Discard events with empty PXD and/or SVD SimHits"""

    def __init__(self, pxd_only=False):
        ''' Initialize the module '''
        super(DiscardEmptyVXD, self).__init__()
        #: PXD only flag
        self.pxd_only = pxd_only
        self.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)

    def event(self):
        ''' Event function '''
        pxdsimhits = Belle2.PyStoreArray("PXDSimHits")
        n = pxdsimhits.getEntries()
        if not self.pxd_only:
            svdsimhits = Belle2.PyStoreArray("SVDSimHits")
            n += svdsimhits.getEntries()

        self.return_value(n > 0)

# Create path
main = create_path()
empty = create_path()

# Add modules
main.add_module("EventInfoSetter", evtNumList=n_events, runList=arguments.run)
beamparameters = add_beamparameters(main, "Y4S")
main.add_module("Gearbox")
main.add_module("Progress")
main.add_module("AafhInput", logLevel=LogLevel.INFO, **aafh_params)
main.add_module("Geometry")
main.add_module("FullSim")
# discard empty events
discard = DiscardEmptyVXD(arguments.pxd_only)
discard.if_false(empty)
main.add_module(discard)
# not using default backgound.add_output as we only save PXD and or SVD
main.add_module("BeamBkgTagSetter", backgroundType=bkgtype, realTime=realtime, specialFor=pxd)
main.add_module("RootOutput", outputFileName=output_filename, branchNames=branches)
# Process events
process(main)

print(statistics)
