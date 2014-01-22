#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
# from simulation import register_simulation
# from reconstruction import register_reconstruction

from setup_vxdtf import setup_vxdtf

set_log_level(LogLevel.WARNING)

argvs = sys.argv
argc = len(argvs)

print argvs[1]
print argvs[2]
print argvs[3]
print argvs[4]
print argc

# field off
fieldOn = False

# Load Geometry module
gearbox = register_module('Gearbox')
# Telescopes, magnetic field, daemon PXD (air), SVD
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v2.xml')

geometry = register_module('Geometry')
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

# PXD rawhit converter
PXDSort = register_module('PXDRawHitSorter')
# If the same pixel appears again in data, merge the two by adding their
# charges, or discard the following occurrences?
# Setting to True may lead to pixels with over-range charge when there are
# several repetitions of data.
PXDSort.param('mergeDuplicates', False)
# True is conservative with respect to downstream processing chain in case
# some parts do not support PXD frames. There is always a single frame,
# though information on number of frames and start rows is propagated.
PXDSort.param('mergeFrames', True)

# PXD clusterizer
PXDClust = register_module('PXDClusterizer')

# PXD DQM module
PXD_DQM = register_module('PXDDQMModule')

# SVD digit sorter
SVDSort = register_module('SVDDigitSorter')
SVDSort.param('mergeDuplicates', False)
# Map of ignored strips
#SVDSort.param('ignoredStripsListName', \
#'testbeam/svd/data/SVD-IgnoredStripsList.xml')

# SVD clusterizer
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
# get a VXDTF with standard settings that writes TrackCands to caTracks
vxdtf = setup_vxdtf('caTracks')

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')

trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('UseClusters', True)

trackfit_dqm = register_module('TrackfitDQM')

# create the main path
main = create_path()

# Add input module
# input = register_module("SeqRootInput")
# input.param ( "inputFileName", "/fcdisk1-1/data/sim/sim-evtgen.sroot")
# main.add_module(input)

# Add Rbuf2Ds
# rbuf2ds = register_module("Rbuf2Ds")
rbuf2ds = register_module('FastRbuf2Ds')
rbuf2ds.param('RingBufferName', argvs[1])
rbuf2ds.param('NumThreads', 2)
main.add_module(rbuf2ds)

# Add DqmHistoManager
hman = register_module('DqmHistoManager')
hman.param('HostName', argvs[2])
hman.param('Port', int(argvs[3]))
# main.add_module(hman)

# Add Progress
progress = register_module('Progress')
main.add_module(progress)

# Add Elapsed Time
elapsed = register_module('ElapsedTime')
elapsed.param('EventInterval', 10000)
main.add_module(elapsed)

# Add Ds2Raw
ds2sample = register_module('Ds2Sample')
ds2sample.param('RingBufferName', argvs[4])
main.add_module(ds2sample)

# Add Gearbox and geometry
main.add_module(gearbox)
main.add_module(geometry)

# PXD: sorter, clusterizer, dqm
main.add_module(PXDSort)
main.add_module(PXDClust)
main.add_module(PXD_DQM)

# SVD: clusterizer, dqm
main.add_module(SVDSort)
main.add_module(SVDClust)
main.add_module(SVD_DQM)

main.add_module(vxdtf)
main.add_module(trackfitter)
main.add_module(vxdtf_dqm)
main.add_module(trackfit_dqm)

# Test seqrootoutput
# output = register_module("SeqRootOutput" )
# output.param ( "outputFileName", "/dev/null" )
# main.add_module(output)

# Run
process(main)
