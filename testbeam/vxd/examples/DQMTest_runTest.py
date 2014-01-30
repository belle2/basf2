#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
# This is testbeam/vxd/scripts/setup_vxdtf.py
from setup_vxdtf import *

# fieldOn = True
fieldOn = True

# flag to use if the PXD is in place and should be used in the Reconstruction
havePXD = False

# Register modules
input = register_module('RootInput')
if fieldOn:
    input.param('inputFileName', 'TBSimulationWBfield.root')
else:
    input.param('inputFileName', 'TBSimulation.root')

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQM-VXD-histo.root')  # File to save histograms

# Report progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# VXD (no Telescopes), and the real PCMAG magnetic field
if havePXD:
    gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v2.xml')
else:
    gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v1.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

# PXD rawhit sorter: convert PXDRawHits to PXDDigits. .
PXDSort = register_module('PXDRawHitSorter')
PXDSort.param('mergeDuplicates', False)
PXDSort.param('mergeFrames', False)
# PXD clusterizer
PXDClust = register_module('PXDClusterizer')
PXDClust.param('TanLorentz', 0.)
# SVD sorter
SVDSort = register_module('SVDDigitSorter')
SVDSort.param('mergeDuplicates', False)
# Use the list of ignored strips. _Verbatim_ path, this is not Gear.
SVDClust = register_module('SVDClusterizer')
SVDClust.param('TanLorentz_holes', 0.)
SVDClust.param('TanLorentz_electrons', 0.)

TelClust = register_module('TelClusterizer')
TelClust.param('Clusters', 'TelClusters')

# PXD DQM module
pxd_dqm = register_module('PXDDQM')

# SVD DQM module
svd_dqm = register_module('SVDDQM')

# TEL DQM module
tel_dqm = register_module('TelDQM')
tel_dqm.param('Clusters', 'TelClusters')

# TB DQM module adds correlations between telescopes and nearest VXD sensors.
telvxd_dqm = register_module('TelxVXD')

# VXDTF:
## parameters:
if havePXD:
    if fieldOn:
        VXDTF = setup_vxdtf('caTracks',
                            ['TB4GeV1TRun500VXD-moreThan1500MeV_VXD'])  # WARNING not working yet!
    else:
        VXDTF = setup_vxdtf('caTracks',
                            ['testBeamMini6GeVVXD-moreThan1500MeV_VXD'])
else:
    if fieldOn:
        VXDTF = setup_vxdtf1T('caTracks',
                              ['TB4GeV1TRun500SVD-moreThan1500MeV_SVD'])
    else:
        VXDTF = setup_vxdtf1T('caTracks',
                              ['TB6GeVNoMagnetSVD-moreThan1500MeV_SVD'])

trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('UseClusters', True)

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'DQMTest_output.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(input)
main.add_module(histo)  # immediately after master module
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(PXDSort)
main.add_module(PXDClust)
main.add_module(SVDSort)
main.add_module(SVDClust)
main.add_module(TelClust)
main.add_module(pxd_dqm)
main.add_module(svd_dqm)
main.add_module(tel_dqm)
main.add_module(telvxd_dqm)
main.add_module(VXDTF)
main.add_module(trackfitter)
main.add_module(vxdtf_dqm)
main.add_module(output)

# Process events
process(main)
