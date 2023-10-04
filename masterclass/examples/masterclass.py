#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from basf2 import Path, process, AfterConditionPath
from modularAnalysis import applyEventCuts
from sys import argv

main = Path()

main.add_module('RootInput')
# basf2 -n100 -i "/group/belle/users/kspenko/data/masterclass/exp26/sub00/*.root" masterclass.py

# Use this only with data (hadron collections) ---------------------------------
event_cuts = "[SoftwareTriggerResult(software_trigger_cut&skim&accept_b_neutral)]"
event_cuts += " or [SoftwareTriggerResult(software_trigger_cut&skim&accept_b_charged)]"
event_cuts += " or [SoftwareTriggerResult(software_trigger_cut&skim&accept_dstar_1)]"
event_cuts += " or [SoftwareTriggerResult(software_trigger_cut&skim&accept_dstar_2)]"
event_cuts += " or [SoftwareTriggerResult(software_trigger_cut&skim&accept_dstar_3)]"
event_cuts += " or [SoftwareTriggerResult(software_trigger_cut&skim&accept_dstar_4)]"
event_cuts += " or [SoftwareTriggerResult(software_trigger_cut&skim&accept_jpsi)]"
event_cuts += " or [SoftwareTriggerResult(software_trigger_cut&skim&accept_kshort)]"
applyEventCuts(event_cuts, path=main)

trigger_skim = main.add_module("TriggerSkim", triggerLines=[
    "software_trigger_cut&skim&accept_b_neutral",
    "software_trigger_cut&skim&accept_b_charged",
    "software_trigger_cut&skim&accept_dstar_1",
    "software_trigger_cut&skim&accept_dstar_2",
    "software_trigger_cut&skim&accept_dstar_3",
    "software_trigger_cut&skim&accept_dstar_4",
    # "software_trigger_cut&skim&accept_jpsi",
    "software_trigger_cut&skim&accept_kshort",
])
trigger_skim.param("expectedResult", 1)
trigger_skim.param("logicMode", "or")
trigger_skim.if_value("==0", Path(), AfterConditionPath.END)


# (Running on grid with custom compiled libraries) In theory this should work---------------------------
# mc_module = register_module(
#     'MasterClass',
#     shared_lib_path='/home/belle2/kspenko/basf2/modules/Linux_x86_64/opt/libmasterclass_modules.so')
# mc_module = register_module(
#     'MasterClass',
#     shared_lib_path='/home/belle2/kspenko/basf2/masterclass/examples/libmasterclass_modules.so')
# print(os.environ["LD_LIBRARY_PATH"])
# mc_module = register_module('MasterClass')
# #, shared_lib_path='./libmasterclass_modules.so')#:./libmasterclass_dataobjects.so')
# main.add_module(mc_module)

masterclass = main.add_module('MasterClass')
if (len(argv) > 1):
    masterclass.param('outputFileName', argv[1])

main.add_module('Progress')
process(main)
