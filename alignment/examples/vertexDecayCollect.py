#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

set_log_level(LogLevel.INFO)

main = create_path()

main.add_module("RootInput")
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe', 'MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD', 'CDC'])

from beamparameters import add_beamparameters
# add_beamparameters(main, "Y4S", None, vertex=[0.01, 0.001, 0.01])

# vertices = ['J/psi:real', 'K_S0:real', 'Lambda0:real']
primary_vertices = ['Z0:mumu']
# main.add_module('UpdateParticleTrackCand', motherListNames=primary_vertices, removeBKLM=True, removeCDC=True)
main.add_module('SetupGenfitExtrapolation', whichGeometry='TGeo', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
# main.add_module('GBLfit', UseClusters=True, addDummyVertexPoint=True, externalIterations=0, StoreFailedTracks=True)
main.add_module('MillepedeCollector', tracks=[], primaryVertices=primary_vertices, calibrateVertex=True)
# main.add_module('GBLdiagnostics')
main.add_module("RootOutput", branchNames=['EventMetaData'])

process(main)
print(statistics)
