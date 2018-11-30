#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor:  Validation of CharmlessHad2Body skim using B -> K*+ rho0

"""
<header>
  <output>../CharmlessHad2BodyCharged.dst.root</output>
  <contact>khsmith@student.unimelb.edu.au</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
import glob

set_random_seed(12345)

# background (collision) files
bg = glob.glob('./BG/[A-Z]*.root')

# create path
charmless2chargedpath = Path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# .dec file for B -> K*+ rho0
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('../../decfiles/dec/1210032007.dec'))
main.add_module(evtgeninput)

# detector simulation
add_simulation(path=charmless2chargedpath)

# reconstruction
add_reconstruction(path=charmless2chargedpath)


# Finally add mdst output
output_filename = "../CharmlessHad2BodyCharged.dst.root"
add_mdst_output(charmless2chargedpath, filename=output_filename)

# process events and print call statistics
process(charmless2chargedpath)
print(statistics)
