#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root</output>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from beamparameters import add_beamparameters
from ROOT import Belle2

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# set the BeamParameters for running at Y(4S)
beamparameters = add_beamparameters(main, "Y4S")
print_params(beamparameters)

# generate Bd->J/psi(-> mu mu) KS0 events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('userDECFile', Belle2.FileSystem.findFile('/decfiles/dec/1111440100.dec'))

main.add_module(evtgeninput)

# detector simulation,  digitisers and clusterisers
add_simulation(main)

# reconstruction
add_reconstruction(main)

# dst output
output = register_module('RootOutput')
output.param('outputFileName', '../Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root')
main.add_module(output)

add_mdst_output(main, True, '../Bd_JpsiKS_mumu_GENSIMRECtoMDST.mdst.root')

# Go!
process(main)

# Print call statistics
print(statistics)
