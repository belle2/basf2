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

main.add_module('EventInfoSetter')

# Use RootInput instead EventInfoSetter if you want to load some event data
# from a ROOT file:
# main.add_module('RootInput')

# create geometry
main.add_module('Gearbox')

# Select only sub-detectors you want to show (e.g. no BeamPipe)
# WARNING: ECL cannot be displayed (will crash due to unsupported volumes)
main.add_module('Geometry', components=['PXD', 'SVD', 'CDC', 'TOP', 'ARICH', 'BKLM', 'EKLM'])

display = register_module('Display')

# save events non-interactively (without showing window)?
display.param('automatic', False)

# change to True to show the full TGeo geometry instead of simplified extract
display.param('fullGeometry', True)

# List of volumes to be hidden (can be re-enabled in Eve panel / Geometry scene.
# The volumes and all their daughters will be hidden.
display.param('hideVolumes', ['BKLM.EnvelopeLogical',
                              'Endcap_1',
                              'Endcap_2',
                              'logicalCDC',
                              'CarbonTube',
                              'PXD.Switcher_refl'])

display.param('deleteVolumes', ['BKLM\\.EnvelopeLogical',
                                '^Endcap_1.*',
                                '^Endcap_2.*',
                                '^logicalCDC_0$',
                                'PXD\\.Switcher.*',
                                'PXD\\.Balcony.*',
                                'PXD\\.Cap.*',
                                'PXD\\.ReadoutChips.*'])

display.param('customGeometryExtractPath', '/home/tadeas/belle2/release/display/examples/geometry_extract.root')


main.add_module(display)

process(main)
# print(statistics(statistics.INIT))
