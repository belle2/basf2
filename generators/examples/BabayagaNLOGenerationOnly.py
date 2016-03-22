#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 100 radiative Bhabha events are generated using the BabayagaNLO
# generator
#
# Example steering file
########################################################

from basf2 import *

# Set the global log level
set_log_level(LogLevel.INFO)

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# Register the BABAYAGA.NLO module
babayaganlo = register_module('BabayagaNLOInput')

# Set the logging level for the BABAYAGA.NLO module to INFO
babayaganlo.set_log_level(LogLevel.INFO)

babayaganlo.param('FinalState', 'ee')  # ee, gg, mm
babayaganlo.param('SearchMax', 10000)
# babayaganlo.param('FMax', 250.0) # for unweighted generation
babayaganlo.param('VacuumPolarization', 'hlmnt')
babayaganlo.param('Order', 'exp')  # born, alpha, exp
babayaganlo.param('Model', 'matched')  # matched
babayaganlo.param('Mode', 'unweighted')  # weighted, unweighted
babayaganlo.param('MinEnergy', 0.1)
babayaganlo.param('Epsilon', 1.e-7)
babayaganlo.param('MaxAcollinearity', 180.)
babayaganlo.param('ScatteringAngleRange', [15.0, 165.0])
babayaganlo.param('DebugEnergySpread', 5.e-3)
babayaganlo.param('VPUncertainty', True)
babayaganlo.param('NPhotons', -1)

babayaganlo.param('UserMode', 'NONE')
# babayaganlo.param('EEMIN', 1.);
# babayaganlo.param('TEMIN', 17.5);
# babayaganlo.param('EEVETO', 0.0);
# babayaganlo.param('TEVETO', 12.5);
# babayaganlo.param('EGMIN', 1.0);
# babayaganlo.param('TGMIN', 1.0);
# babayaganlo.param('EGVETO', 0.0);
# babayaganlo.param('TGVETO', 5.0);

# Create the main path and add the modules
main.add_module("Progress")
main.add_module(babayaganlo)
main.add_module("RootOutput", outputFileName="babayaganlo.root")

main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
