import os
from basf2 import *

#Register modules
input = register_module("RootInput")
input.param('inputFileName', 'TBSimulation.root')

# Histogram manager immediately after master module
histo = register_module("HistoManager")
histo.param("histoFileName", "DQM-VXD-histo.root")  # File to save histograms

# Report progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# VXD (no Telescopes), and the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
geometry.param('components', ['TB'])

# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
PXDClust.param('TanLorentz', 0.)

SVDClust = register_module('SVDClusterizer')
SVDClust.param('TanLorentz_holes', 0.)
SVDClust.param('TanLorentz_electrons', 0.)

# PXD DQM module
pxd_dqm = register_module("PXDDQM")

# SVD DQM module
svd_dqm = register_module("SVDDQM")

#Create paths
main = create_path()

#Add modules to paths
main.add_module(input)
main.add_module(histo)  # immediately after master module
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(PXDClust)
main.add_module(SVDClust)
main.add_module(pxd_dqm)
main.add_module(svd_dqm)

#Process events
process(main)
