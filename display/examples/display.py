#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#
# Opens a .root/.sroot file and shows MCParticles,
# SimHits and Tracks using the Display module.
# Usage:
#  basf2 display/example/display.py -i MyInputFile.root [-- --play]
#
# Note: this file is also used by the 'b2display' command,
# so the following is also possible:
#  b2display MyInputFile.root
#
# If you want custom settings for b2display, you thus only need to
# edit this steering file.

import argparse
import basf2 as b2
from ROOT import Belle2

ap = argparse.ArgumentParser()
ap.add_argument("--play", action='store_true', help="Start event display advancing through events.")
args = ap.parse_args()

# create paths
main = b2.create_path()

# Get type of input file to decide, which input module we want to use
input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    rootinput = b2.register_module('SeqRootInput')
else:
    rootinput = b2.register_module('RootInput')

# create geometry
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
# new ECL geometry contains custom objects that cannot be converted to TGeo
# add MagneticField off B-field (also greatly speeds up startup)
geometry.param('excludedComponents', ['ECL'])

main.add_module(rootinput)
main.add_module(gearbox)
main.add_module(geometry)

display = b2.register_module('Display')
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

# set to any downloaded/created extract (=simplified geometry + 2D projections)
# Default extract (for Phase 3) is stored in display/data
display.param('customGeometryExtractPath', '')

# Objects which are to be hidden (can be manually re-enabled in tree view).
# Names correspond to the object names in the 'Event'. (Note that this won't
# work for objects somewhere deep in the tree, only for those immediately
# below 'Event'.
display.param('hideObjects', [])

# should events be advanced on startup?
if args.play:
    display.param('playOnStartup', True)

main.add_module(display)

b2.process(main)
# print(statistics(statistics.INIT))
