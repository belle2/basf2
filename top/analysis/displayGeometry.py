#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

# ---------------------------------------------------------------------------
# Shows the TOP counter using event display
#
# Usage:  basf2 top/analysis/displayGeometry.py
#
# The filename of the main xml file can be specified as an optional argument
#   example: basf2 top/analysis/displayGeometry.py top/TOPSpareModule.xml
#            (hint: omit 'data' from the path)
# ---------------------------------------------------------------------------

# create path
main = create_path()

# set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# gearbox
gearbox = register_module('Gearbox')
argvs = sys.argv
if len(argvs) > 1:
    fileName = argvs[1]
    gearbox.param('fileName', fileName)
main.add_module(gearbox)

# geometry (TOP only)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# event display
display = register_module('Display')
display.param('showMCInfo', False)
display.param('assignHitsToPrimaries', False)
display.param('showAllPrimaries', False)
display.param('showCharged', False)
display.param('showNeutrals', False)
display.param('showRecoTracks', False)
display.param('showCDCHits', False)
display.param('showTrackLevelObjects', False)
display.param('options', 'D')  # default
display.param('automatic', False)
display.param('fullGeometry', True)
display.param('hideObjects', [])
main.add_module(display)

process(main)
