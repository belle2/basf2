#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#####################################################################
#
# History:
# 21 Jul 2015, Commit
#
#####################################################################


"""
<header>
  <input>EvtGenSimRec.root</input>
  <output>SVDValidationOutput.root</output>
  <output>SVDValidationTTree.root</output>
  <output>SVDValidationTTreeStrip.root</output>
  <output>SVDValidationTTreeSpacePoint.root</output>
  <output>SVDValidationTTreeSimhit.root</output>
  <contact>G. Caria, gcaria@student.unimelb.edu.au</contact>
  <description>This is the SVD validation steering file.</description>
</header>
"""

from basf2 import *
# Individual validation packages
from SVDValidationTTree import *
from SVDValidationTTreeStrip import *
from SVDValidationTTreeSimhit import *
from SVDValidationTTreeSpacePoint import *

set_random_seed(12345)

main = create_path()

# Using 1000 EvtGen events already simulated and reconstructed
input = register_module('RootInput')
input.param('inputFileName', '../EvtGenSimRec.root')
main.add_module(input)

# Gearbox and Geometry modules need to be registered anyway

# Load parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Create geometry
geometry = register_module('Geometry')
# Select subdetectors to be built
geometry.param('components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD'])
# geometry.param("excludedComponents", "MagneticField")
main.add_module(geometry)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# SVD validation modules
svdvalidation = SVDValidationTTree()
main.add_module(svdvalidation)
svdvalidationstrip = SVDValidationTTreeStrip()
main.add_module(svdvalidationstrip)
svdvalidationspacepoint = SVDValidationTTreeSpacePoint()
main.add_module(svdvalidationspacepoint)
svdvalidationsimhit = SVDValidationTTreeSimhit()
main.add_module(svdvalidationsimhit)

process(main)

# Print call statistics
print(statistics)
