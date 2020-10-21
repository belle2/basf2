#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: e+e- --> e+e-(ISR)

"""
<header>
  <output>../TwoTrackElectronsForLuminosity.dst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=1000)

# Register the BABAYAGA.NLO module
babayaganlo = basf2.register_module('BabayagaNLOInput')

# Set the logging level for the BABAYAGA.NLO module to INFO
babayaganlo.set_log_level(basf2.LogLevel.INFO)

babayaganlo.param('FinalState', 'ee')  # ee, gg, mm
babayaganlo.param('DebugEnergySpread', 5.e-3)
babayaganlo.param('ScatteringAngleRange', [35.0, 145.0])
babayaganlo.param('MinEnergy', 0.01)
babayaganlo.param('MaxAcollinearity', 180.)

babayaganlo.param('Mode', 'unweighted')  # weighted, unweighted
babayaganlo.param('Order', 'exp')  # born, alpha, exp
babayaganlo.param('Model', 'matched')  # matched
babayaganlo.param('VacuumPolarization', 'hlmnt')
babayaganlo.param('VPUncertainty', True)
babayaganlo.param('NPhotons', -1)
babayaganlo.param('Epsilon', 1.e-7)
babayaganlo.param('SearchMax', 1000000)
# babayaganlo.param('FMax', 1.e+6) # for unweighted generation

babayaganlo.param('UserMode', 'NONE')
# babayaganlo.param('EEMIN', 1.);
# babayaganlo.param('TEMIN', 17.5);
# babayaganlo.param('EEVETO', 0.0);
# babayaganlo.param('TEVETO', 12.5);
# babayaganlo.param('EGMIN', 1.0);
# babayaganlo.param('TGMIN', 1.0);
# babayaganlo.param('EGVETO', 0.0);
# babayaganlo.param('TGVETO', 5.0);

main.add_module(babayaganlo)

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

# Finally add mdst output
output_filename = "../TwoTrackElectronsForLuminosity.dst.root"
add_mdst_output(main, filename=output_filename)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
