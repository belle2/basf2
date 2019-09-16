#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor:  B to KS pi+ pi- gamma  decay


"""
<header>
  <output>../TCPV.dst.root</output>
  <contact>rrasheed@desy.de</contact>
</header>
"""

import basf2 as b2
import mdst as mdst
import generators as ge
import simulation as si
import reconstruction as re
import modularAnalysis as ma
from ROOT import Belle2
import glob


# Defining custom path
tcpvskimpath = b2.Path()

# specify number of events to be generated
ma.setupEventInfo(noEvents=1000, path=tcpvskimpath)

ge.add_evtgen_generator(path=tcpvskimpath,
                        finalstate='signal',
                        signaldecfile=b2.find_file(
                            'skim/validation/dectcpv.dec'))


# detector simulation
# add_simulation(main, bkgfiles=bg)
si.add_simulation(path=tcpvskimpath)

# reconstruction
re.add_reconstruction(path=tcpvskimpath)


# Finally add mdst output
output_filename = "../TCPV.dst.root"
mdst.add_mdst_output(path=tcpvskimpath, mc=True, filename=output_filename)


# process events and print call statistics
b2.process(tcpvskimpath)
print(b2.statistics)
