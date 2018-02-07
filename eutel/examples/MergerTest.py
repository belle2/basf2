#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# Set the log level to show only warning, error and, fatal messages
# otherwise there's gonna be a segfault when python exits
set_log_level(LogLevel.WARNING)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'Tel-histo.root')  # File to save histograms

# Load parameters from xml
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v2.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
geometry.param('components', ['TB'])

# input
input = register_module('SeqRootInput')
input.param('inputFileName', '../vxdtb/data/e0000r000391.sroot')
# data reader
dataMerger = register_module('TelDataMerger')
# use corresponding telescope data file
dataMerger.param('inputFileName', '../vxdtb/data/run000070.raw')
dataMerger.param('bufferSize', 2000)
dataMerger.param('eutelPlaneNrs', [
    4,
    3,
    2,
    0,
    1,
    5,
    ])
# This produces a lot of console output. Once you're sure the merger works,
# set log level WARNING to disable it. However, when starting with a new file,
# look at the output to make sure that the merger works correctly.
dataMerger.set_log_level(LogLevel.WARNING)

# PXD unpacker
PXDUnpack = register_module('PXDUnpacker')
# PXD rawhit sorter: convert PXDRawHits to PXDDigits. .
PXDSort = register_module('PXDRawHitSorter')
PXDSort.param('mergeDuplicates', False)
PXDSort.param('mergeFrames', True)
# PXD clusterizer
PXDClust = register_module('PXDClusterizer')
PXDClust.param('TanLorentz', 0.)
# PXD DQM modules
PXD_DQM = register_module('PXDDQM')
raw_pxd_dqm = register_module('pxdRawDQM')

# SVD Unpacker
svdUnpacker = register_module('SVDUnpacker')
svdUnpacker.param('rawSVDListName', 'RawSVDs')
svdUnpacker.param('svdDigitListName', 'SVDDigits')
svdUnpacker.param('xmlMapFileName',
                  'testbeam/vxd/data/SVD-OnlineOfflineMap.xml')

# SVD Digit sorter
SVDSort = register_module('SVDDigitSorter')
SVDSort.param('mergeDuplicates', False)
# No gearbox conventions, VERBATIM path!
# SVDSort.param('ignoredStripsListName', 'testbeam/vxd/data/SVD-IgnoredStripsList.xml')
# SVD clusterizer
SVDClust = register_module('SVDClusterizer')
SVDClust.param('TanLorentz_holes', 0.)
SVDClust.param('TanLorentz_electrons', 0.)
# SVDClust.param('applyTimeWindow', True)
# SVDClust.param('triggerTime', -20.0)
# SVDClust.param('acceptanceWindowSize', 20.0)

# SVD DQM module
svd_dqm = register_module('SVDDQM')

TelClust = register_module('TelClusterizer')
TelClust.param('Clusters', 'TelClusters')

# TEL DQM module
tel_dqm = register_module('TelDQM')
tel_dqm.param('Clusters', 'TelClusters')

# TB DQM module adds correlations between telescopes and nearest VXD sensors.
telvxd_dqm = register_module('TelxVXD')
telvxd_dqm.param('UseSpacePoints', 1)

progress = register_module('Progress')
dataWriter = register_module('RootOutput')
dataWriter.param('outputFileName', 'TelMergerOutput.root')

main = create_path()
main.add_module(input)
main.add_module(histo)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(dataMerger)
main.add_module(PXDUnpack)
main.add_module(PXDSort)
main.add_module(PXDClust)
main.add_module(PXD_DQM)
main.add_module(svdUnpacker)
main.add_module(SVDSort)
main.add_module(SVDClust)
main.add_module(svd_dqm)
main.add_module(TelClust)
main.add_module(tel_dqm)
main.add_module(telvxd_dqm)
main.add_module(dataWriter)

process(main)
