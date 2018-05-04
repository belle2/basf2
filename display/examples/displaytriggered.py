#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Opens a .root/.sroot file and shows MCParticles,
# SimHits and Tracks using the Display module. Only
# events which were triggered by the HLT are displayed.
# Usage:
#  basf2 display/example/displaytrigger.py -i MyInputFile.root

from basf2 import *
import ROOT
from ROOT import Belle2


class DisplayHLTTags(Module):
    """Test DisplayData"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

        Belle2.PyStoreObj("DisplayData").registerInDataStore()

    def event(self):
        """reimplementation of Module::event()."""

        displayData = Belle2.PyStoreObj("DisplayData")
        displayData.create()

        print("HLT Tags:")
        displayData.obj().addLabel('HLT Tags:', ROOT.TVector3(200, 220, -300))
        trigger_result = Belle2.PyStoreObj('SoftwareTriggerResult')
        dz_pos = 0
        for name, result in trigger_result.getResults():
            if result == 1 and name.startswith('software_trigger_cut&hlt&accept_'):
                name = name[32:]
                displayData.obj().addLabel(name, ROOT.TVector3(200, 200 - dz_pos, -300))
                dz_pos += 15
                print(name)


# create paths
main = create_path()

# Get type of input file to decide, which input module we want to use
input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    rootinput = register_module('SeqRootInput')
else:
    rootinput = register_module('RootInput')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
# new ECL geometry contains custom objects that cannot be converted to TGeo
# add MagneticField off B-field (also greatly speeds up startup)
geometry.param('excludedComponents', ['ECL'])

main.add_module(rootinput)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(DisplayHLTTags())

display = register_module('Display')
# --- MC options ---
# Should Monte Carlo info be shown? (MCParticles, SimHits)
display.param('showMCInfo', True)

# should hits always be assigned to a particle with c_PrimaryParticle flag?
display.param('assignHitsToPrimaries', False)

# show all primary MCParticles?
display.param('showAllPrimaries', True)

# show all charged MCParticles?
display.param('showCharged', True)

# show all neutral MCParticles?
display.param('showNeutrals', True)


# --- Intermediate options ---
# show track candidates?
# You most likely don't want this unless you are a tracking developer
display.param('showRecoTracks', False)

# directly show CDCHits as drift cylinders (shortened, z position = 0)
display.param('showCDCHits', False)

# show trigger objects?
display.param('showTriggerObjects', False)


# --- Track options ---
# show tracks, vertices, eclgammas?
display.param('showTrackLevelObjects', True)

# The options parameter is a combination of:
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# M draw track markers - intersections of track with detector planes
# P draw detector planes
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
#
# only makes sense when showTrackLevelObjects/showTrackCandidates is used
display.param('options', 'MH')  # default

# --- Other options ---

# save events non-interactively (without showing window)?
display.param('automatic', False)

# change to True to show the full TGeo geometry instead of simplified extract
display.param('fullGeometry', False)

# Objects which are to be hidden (can be manually re-enabled in tree view).
# Names correspond to the object names in the 'Event'. (Note that this won't
# work for objects somewhere deep in the tree, only for those immediately
# below 'Event'.
display.param('hideObjects', [])

# this path will only be executed for events which satfisy the trigger skim
triggered_event_path = Path()
triggered_event_path.add_module(display)

trigger_skim_module = register_module("TriggerSkim")
# add whatever HLT/Calib trigger lines you would like to select here
trigger_skim_module.param('triggerLines', ['software_trigger_cut&hlt&accept_mumu_2trk',
                                           'software_trigger_cut&hlt&accept_bhabha',
                                           'software_trigger_cut&hlt&accept_hadron'])
# if any of the trigger lines are true, run the display path
trigger_skim_module.if_value("==1", triggered_event_path, AfterConditionPath.CONTINUE)

main.add_module(trigger_skim_module)

process(main)
