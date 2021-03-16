#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script determines the time spent in each part of the simulation. It does
that by assigning regions to the geometry created by each creator and then
measuring the time spent in these regions using the G4VSteppingVerbose
facilities provided by Geant4. These get compiled out in the opt version of the
externals so this only works with the debug externals. It was intended as a
validation script but due to this problem it was moved to examples

<header>
  <contact>ritter</contact>
  <description>
      Generate detailed timing information of the simulation when simulating
      100 EvtGen events.
      WARNING: Does not work in parallel processing mode
  </description>
</header>
"""

import os
import basf2 as b2
import ROOT

if os.environ.get("BELLE2_EXTERNALS_OPTION", "opt") != "debug":
    b2.B2FATAL("This script needs to be run with debug externals, otherwise it "
               "cannot determine the time spent in each volume.")

# limit output
b2.logging.log_level = b2.LogLevel.WARNING
# disable multi processing
b2.set_nprocesses(0)

main = b2.create_path()
# create 100 events
main.add_module("EventInfoSetter", evtNumList=[100])
# using standard evtgen
main.add_module("EvtGenInput")
# and parameters
main.add_module("Gearbox")
# and the geometry with assigned regions for each creator
main.add_module("Geometry", assignRegions=True)
# as well as the simulation
main.add_module("FullSim")
# including the timing module
main.add_module("FullSimTiming", rootFile="EvtGenTiming.root", logLevel=b2.LogLevel.INFO)

# and run it
b2.process(main)


def add_info(obj, title, text):
    """Add a description item to a TH object"""
    obj.GetListOfFunctions().Add(ROOT.TNamed(title, text))


# now open the created root file and update some things
root_file = ROOT.TFile("EvtGenTiming.root", "UPDATE")
timing = root_file.Get("SimulationTiming")
# like set the minimum to zero
timing.SetMinimum(0)
# lets update the "DefaultRegionOfTheWorld" name to something more useful
for i in range(timing.GetNbinsX()):
    if timing.GetXaxis().GetBinLabel(i + 1) == "DefaultRegionForTheWorld":
        timing.GetXaxis().SetBinLabel(i + 1, "Top Volume")

# add the required descriptions
add_info(timing, "Description",
         "Time spent in each sub detector when simulating EvtGen events "
         "(default settings). This includes tracking through the volume, "
         "calculating of all physics processes and calling the sensitive "
         "detector implementations (if any). It does not include the "
         "overall Geant4/framework overhead (event action, generation, "
         "stacking action, tracking action, stepping action and so forth) "
         "<br>The errors are standard deviations of the stepping time per "
         "event/region")
add_info(timing, "Check", "Dunno, mostly for informational purpose but "
         "large deviations might point to problematic geometry/sensitive "
         "detector updates")
add_info(timing, "Contact", "ritter")

timing.Write()
root_file.Close()
