#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Opens a .root/.sroot file and shows MCParticles,
# SimHits and Tracks using the Display module.
# Usage:
#  basf2 display/example/display.py -i MyInputFile.root
#
# Note: this file is also used by the 'b2display' command,
# so the following is also possible:
#  b2display MyInputFile.root
#
# If you want custom settings for b2display, you thus only need to
# edit this steering file.

from basf2 import *
from ROOT import Belle2

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
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')
geometry = register_module('Geometry')
# new ECL geometry contains custom objects that cannot be converted to TGeo
# add MagneticField off B-field (also greatly speeds up startup)
geometry.param('excludedComponents', ['ECL', 'TOP', 'ARICH'])
# geometry.param('excludedComponents', ['ECL'])

main.add_module(rootinput)
main.add_module(gearbox)
main.add_module(geometry)

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
display.param('showRecoTracks', True)

# directly show CDCHits as drift cylinders (shortened, z position = 0)
display.param('showCDCHits', False)


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
display.param('fullGeometry', True)

# Objects which are to be hidden (can be manually re-enabled in tree view).
# Names correspond to the object names in the 'Event'. (Note that this won't
# work for objects somewhere deep in the tree, only for those immediately
# below 'Event'.
display.param('hideObjects', [])

main.add_module(display)

process(main)
# print(statistics(statistics.INIT))
