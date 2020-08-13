#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor:  Validation of CharmlessHad3Body skim using B -> K_S0 pi+ pi0

"""
<header>
  <output>../CharmlessHad3BodyCharged.dst.root</output>
  <contact>khsmith@student.unimelb.edu.au</contact>
</header>
"""

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
import glob

b2.set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create path
charmless3chargedpath = b2.Path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
charmless3chargedpath.add_module(eventinfosetter)

# .dec file for B -> K_S0 pi+ pi0
evtgeninput = b2.register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1210032100.dec'))
charmless3chargedpath.add_module(evtgeninput)

# detector simulation
add_simulation(path=charmless3chargedpath)

# reconstruction
add_reconstruction(path=charmless3chargedpath)


# Finally add mdst output
output_filename = "../CharmlessHad3BodyCharged.dst.root"
add_mdst_output(charmless3chargedpath, filename=output_filename)

# process events and print call statistics
b2.process(charmless3chargedpath)
print(b2.statistics)
