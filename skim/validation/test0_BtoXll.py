#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor:  B to K l+ l-  decay


"""
<header>
  <output>../BtoXll.dst.root</output>
  <contact>soumen.halder@desy.de</contact>
</header>
"""

import basf2 as b2
import mdst as mdst
import generators as ge
import simulation as si
import reconstruction as re
import modularAnalysis as ma


# Defining custom path
my_path = b2.Path()

# specify number of events to be generated
ma.setupEventInfo(noEvents=1000, path=my_path)

ge.add_evtgen_generator(path=my_path,
                        finalstate='signal',
                        signaldecfile=b2.find_file(
                            'decfiles/dec/1210230011.dec'))


# detector simulation
# add_simulation(main, bkgfiles=bg)
si.add_simulation(path=my_path)

# reconstruction
re.add_reconstruction(path=my_path)


# Finally add mdst output
output_filename = "../BtoXll.dst.root"
mdst.add_mdst_output(path=my_path, mc=True, filename=output_filename)


# process events and print call statistics
b2.process(my_path)
print(b2.statistics)
