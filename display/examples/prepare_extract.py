#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------
# Create a geometry extract (needed for R-Phi and R-Z projections)
# ----------------------------------------------------------------
#
# PLEASE READ COMMENTS IN THIS SCRIPT
#
# Geometry extract is a simplified and small version of the geometry
# stored in a ROOT file and loaded by default (from display/data/geometry_extract.root)
# It is needed for R-Phi and R-Z projection (empty for full geometry).
# It is faster to display and manipulate (zoom, rotation) than full geometry
#
# This script will open Display (by default, read below) where one can:
# 1) Remove volumes to have reasonably-sized extract
#    - Go to Eve panel -> Scenes -> Geometry scene
#    - Select volumes to be removed completely from the extract by right-clicking
#      on them and selecting 'Destroy' from the pop-up menu
# 2) Hide volumes for better visibility (just un-check their check-box).
#    These can be later re-enabled in the Display.
# 3) Change volume colors, transparency etc. (only in the Display UI)
# 4) Save the extract from window menu by clicking on Scene -> Save geometry extract
#    It will be saved in current directory with name 'geometry_extract.root'
#
# NOTE: 1) and 2) can be done also automatically from the script (read below)
# NOTE: You are supposed to review / slightly modify the visual appearance in
#       the Display UI. But using the parameters in this script can save you from
#       lot of tedious work of destroying hundreds of volumes by hand

from basf2 import *
from ROOT import Belle2

# create paths
main = create_path()

# Use RootInput instead EventInfoSetter (not both!) if you want
# to load some event data from a ROOT file
main.add_module('EventInfoSetter')
# main.add_module('RootInput')

# You can specifi different geometry here, e.g. for Phase2, do:
# main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')
main.add_module('Gearbox')

# Select only sub-detectors you want to show (e.g. no BeamPipe)
# WARNING: ECL cannot be displayed (will crash due to unsupported volumes)
main.add_module('Geometry', components=['PXD', 'SVD', 'CDC', 'TOP', 'ARICH', 'BKLM', 'EKLM'])

display = register_module('Display')

# Has to be True to show the full TGeo geometry instead of simplified extract
# which you want to prepare with this script
display.param('fullGeometry', True)

# List of volumes to be hidden (can be re-enabled in Eve panel / Geometry scene)
# The volumes and all their daughters will be hidden.
# NOTE: Volume names in the Eve panel have usually a suffix ('_1' etc. which you have to omit here)
#       For example 'BKLM.EnvelopeLogical' is named 'BKLM.EnvelopeLogical_1' but here you need to specify
#       them without the suffix (their visualisation is changed directly in geometry in memory - which
#       has no effect to e.g. simulation, it is only for visualisation - so many have the same name)
#       For example 'PXD.Switcher_refl' will hide all 'PXD.Switcher_refl_#' you can see in the Eve panel.
display.param('hideVolumes', ['BKLM.EnvelopeLogical',
                              'Endcap_1',
                              'Endcap_2',
                              'logicalCDC',
                              'CarbonTube',
                              'PXD.Switcher_refl'])

# List of volumes to be deleted. This time the suffix is needed.
# NOTE: These are REGULAR EXPRESSIONS from ROOT, see:
#       https://root.cern.ch/root/html534/TPRegexp.html
#
# Some usefull examples:
#
# - You need to escape special characters like '.' (dot) by '\\'
#   For example to select all volumes which contain 'PXD.ReadoutChips', use:
#   'PXD\\.ReadoutChips'
#
# - To select exactly the volume with given name, e.g. 'logicalCDC_0', use:
#   '^logicalCDC_0$'
#
# - To select any volume which contains some string, e.g. 'Encap_1'. 'LayerEndcap_2' etc., use simply:
#   'Endcap'
#
# - To select volumes starting with some string, e.g. 'Endcap_1', 'Endcap_2' etc., use:
#   '^Endcap'
#
# - '.*' means any or no characters, so e.g. to remove volumes which contain
#   'PXD.Switcher', 'PXD.SwitcherChip', 'Switcher' etc., you could use:
#   'Switcher.*'
#
# A B2INFO message is issued for every volume deleted. Please check the output is what you expect.
#
# Follows a default list used to create Phase 3 extract:
display.param('deleteVolumes', ['BKLM\\.EnvelopeLogical',
                                '^Endcap_1.*',
                                '^Endcap_2.*',
                                '^logicalCDC_0$',
                                'PXD\\.Switcher.*',
                                'PXD\\.Balcony.*',
                                'PXD\\.Cap.*',
                                'PXD\\.ReadoutChips.*'])

# To use custom extract afterwards in Display, you need to set
# fullGeometry=False (default) and change the path to the extract used (in your own display script):
#
# display.param('customGeometryExtractPath', 'path/to/extract/geometry_extract.root')

main.add_module(display)

process(main)
# print(statistics(statistics.INIT))
