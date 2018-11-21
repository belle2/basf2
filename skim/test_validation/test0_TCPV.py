#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor:  B to KS pi+ pi- gamma  decay


"""
<header>
  <output>../TCPV.dst.root</output>
  <contact>rrasheed@desy.de</contact>
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

# crea
tcpvskimpath = Path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
tcpvskimpath.add_module(eventinfosetter)

evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('skim/test_validation/TCPV.dec'))
tcpvskimpath.add_module(evtgeninput)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(path=tcpvskimpath)

# reconstruction
add_reconstruction(path=tcpvskimpath)


# Finally add mdst output
output_filename = "../TCPV.dst.root"
add_mdst_output(tcpvskimpath, filename=output_filename)

# process events and print call statistics
process(tcpvskimpath)
print(statistics)
