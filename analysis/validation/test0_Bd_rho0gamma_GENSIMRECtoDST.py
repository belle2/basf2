#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>Bd_rho0gamma_GENSIMRECtoDST.dst.root</output>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_random_seed(12345)

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# generate Bd->Kstgamma events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/generators/evtgen/decayfiles/Bd_rho0gamma,pipi.dec')

main.add_module(evtgeninput)

# detector simulation,  digitisers and clusterisers
add_simulation(main)

# reconstruction
add_reconstruction(main)

# write out the full dst output
output = register_module('RootOutput')
output.param('outputFileName', '../Bd_rho0gamma_GENSIMRECtoDST.dst.root')
main.add_module(output)

# Go!
process(main)

# Print call statistics
print statistics
