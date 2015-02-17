#!/usr/bin/env python
# coding:utf8
"""
<header>
  <contact>ritter</contact>
  <description>
      Generate detailed timing information of the simulation when simulating
      100 EvtGen events.
      WARNING: Does not work in parallel processing mode
  </description>
</header>
"""
from basf2 import *
import ROOT

logging.log_level = LogLevel.WARNING

main = create_path()
main.add_module("EventInfoSetter", evtNumList=[100])
main.add_module("Gearbox")
main.add_module("Geometry", assignRegions=True)
main.add_module("EvtGenInput")
main.add_module("FullSim")
main.add_module("FullSimTiming", rootFile="EvtGenTiming.root", logLevel=LogLevel.INFO)
process(main)


def add_info(obj, title, text):
    """Add a description item to a TH object"""
    obj.GetListOfFunctions().Add(ROOT.TNamed(title, text))

root_file = ROOT.TFile("EvtGenTiming.root", "UPDATE")
timing = root_file.Get("SimulationTiming")
timing.SetMinimum(0)
# lets update the "DefaultRegionOfTheWorld" name to something more useful
for i in range(timing.GetNbinsX()):
    if timing.GetXaxis().GetBinLabel(i + 1) == "DefaultRegionForTheWorld":
        timing.GetXaxis().SetBinLabel(i + 1, "Top Volume")

# add the required descriptions
add_info(timing, "Description",
         "Time spent in each sub detector when simulating EvtGen events " +
         "(default settings). This includes tracking through the volume, " +
         "calculating of all physics processes and calling the sensitive " +
         "detector implementations (if any). It does not include the " +
         "overall Geant4/framework overhead (event action, generation, " +
         "stacking action, tracking action, stepping action and so forth) " +
         "<br>The errors are standard deviations of the stepping time per " +
         "event/region")
add_info(timing, "Check", "Dunno, mostly for informational purpose but " +
         "large deviations might point to problematic geometry/sensitive " +
         "detector updates")
add_info(timing, "Contact", "ritter")

timing.Write()
root_file.Close()
