#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
# This is testbeam/vxd/scripts/setup_vxdtf.py
from setup_vxdtf import setup_vxdtf
#from simulation import register_simulation
#from reconstruction import register_reconstruction

set_log_level(LogLevel.ERROR)

argvs = sys.argv
argc = len(argvs)

#print argvs[1]
#print argvs[2]
#print argvs[3]
#print argvs[4]
#print argc

# B-field
# fieldOn = True
fieldOn = False

# flag to use if the PXD is in place and should be used in the Reconstruction
havePXD = True

# data file
inputFileName = '/path/to/srrot/files/e0000r000rrr.sroot'

# histogram file
histoFileName = 'DQM-VXD-histo.root'

# which geometry
if havePXD:
    geometry_file = 'testbeam/vxd/FullTelescopeVXDTB_v2.xml'
else:
    geometry_file = 'testbeam/vxd/FullTelescopeVXDTB_v1.xml'


# Register modules
input = register_module('SeqRootInput')
input.param('inputFileName', inputFileName)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', histoFileName)  # File to save histograms

# Add Rbuf2Ds
#rbuf2ds = register_module("Rbuf2Ds")
#rbuf2ds = register_module("FastRbuf2Ds")
#rbuf2ds.param("RingBufferName", argvs[1])
#rbuf2ds.param("NumThreads", 1 )
#main.add_module(rbuf2ds)

# Add DqmHistoManager
#hman = register_module("DqmHistoManager")
#hman.param("HostName", argvs[2] );
#hman.param("Port", int(argvs[3]) );
#main.add_module(hman)

# Add Ds2Raw
#ds2sample = register_module("Ds2Sample")
#ds2sample.param("RingBufferName", argvs[4])
#main.add_module(ds2sample)

# Report progress of processing
progress = register_module('Progress')

# Load Geometry module
gearbox = register_module('Gearbox')
# Telescopes, magnetic field, PXD, SVD
# Need to select the
gearbox.param('fileName', geometry_file)

geometry = register_module('Geometry')
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

if havePXD:
    # PXD unpacker
    PXDUnpack = register_module('PXDUnpacker')
    PXDUnpack.param('DHHCmode', True)
    PXDUnpack.param('HeaderEndianSwap', True)
    # PXD rawhit sorter: convert PXDRawHits to PXDDigits. .
    PXDSort = register_module('PXDRawHitSorter')
    PXDSort.param('mergeDuplicates', False)
    PXDSort.param('mergeFrames', True)
    # PXD clusterizer
    PXDClust = register_module('PXDClusterizer')
    if fieldOn:
        PXDClust.param('TanLorentz', 0.16)
    else:
        PXDClust.param('TanLorentz', 0.)
        # PXD DQM modules
    PXD_DQM = register_module('PXDDQM')
    raw_pxd_dqm = register_module('pxdRawDQM')

SVDUnpack = register_module('SVDUnpacker')
SVDUnpack.param('xmlMapFileName', 'testbeam/vxd/data/SVD-OnlineOfflineMap.xml')

SVDSort = register_module('SVDDigitSorter')
SVDSort.param('mergeDuplicates', False)

SVDClust = register_module('SVDClusterizer')
if fieldOn:
    SVDClust.param('TanLorentz_holes', 0.052)
    SVDClust.param('TanLorentz_electrons', 0.)
else:
    SVDClust.param('TanLorentz_holes', 0.)
    SVDClust.param('TanLorentz_electrons', 0.)
# SVD DQM module
SVD_DQM = register_module('SVDDQM')

# VXDTF:

# Depending on whether we have the PXD, we include it in trackfinding or don't
if havePXD:
    VXDTF = setup_vxdtf('caTracks', ['testBeamMini6GeVVXD-moreThan1500MeV_VXD'
                        ])
else:
    VXDTF = setup_vxdtf('caTracks', ['testBeamMini6GeVSVD-moreThan1500MeV_SVD'
                        ])

# VXDTF DQM module
VXDTF_DQM = register_module('VXDTFDQM')

trackfitter = register_module('GenFitter')
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('UseClusters', True)
trackfitter.param('NMinIterations', 3)
trackfitter.param('NMaxIterations', 3)

trackfit_dqm = register_module('TrackfitDQM')
trackfit_dqm.param('GFTrackCandidatesColName', 'caTracks')

# Create paths
main = create_path()

# Add modules to paths
#main.add_module(rbuf2ds)
main.add_module(input)
main.add_module(histo)
#main.add_module(hman)  # immediately after master module
main.add_module(gearbox)
main.add_module(geometry)
#main.add_module(ds2sample)
if havePXD:
    main.add_module(PXDUnpack)
    main.add_module(PXDSort)
    main.add_module(PXDClust)
    main.add_module(raw_pxd_dqm)
    main.add_module(PXD_DQM)

main.add_module(SVDUnpack)
main.add_module(SVDSort)
main.add_module(SVDClust)
main.add_module(SVD_DQM)
main.add_module(VXDTF)
main.add_module(trackfitter)
main.add_module(VXDTF_DQM)
main.add_module(trackfit_dqm)
main.add_module(progress)

# Process events
process(main)
