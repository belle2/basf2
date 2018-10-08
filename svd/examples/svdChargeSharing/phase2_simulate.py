#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# generic BBbar events using EvtGen
# Example steering file

from basf2 import *

# main path
main = create_path()
# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=10000)

# EvtGen
evtgen = register_module('EvtGenInput')
# evtgen.set_log_level(LogLevel.INFO)

gearbox = register_module('Gearbox')
geomfile = '/geometry/Beast2_phase2.xml'
if geomfile != 'None':
    gearbox.param('fileName', geomfile)

main.add_module(gearbox)
geometry = register_module('Geometry')
geometry.param('components', ['SVD', 'CDC', 'PXD'])
main.add_module(geometry)

main.add_module(evtgen)

main.add_module('FullSim', StoreAllSecondaries=True)

main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('CDCDigitizer')

main.add_module("Progress")
main.add_module(
    'RootOutput',
    outputFileName=str(
        sys.argv[1]),
    branchNames=[
        "MCParticles",
        "CDCHits",
        "MCParticlesToCDCHits",
        "MCParticlesToSVDTrueHits",
        "SVDShaperDigits",
        "SVDShaperDigitsToMCParticles",
        "SVDShaperDigitsToSVDTrueHits",
        "SVDSimHits",
        "SVDTrueHits",
        "SVDTrueHitsToSVDSimHits"])

# generate events
process(main)

# show call statistics
print(statistics)
