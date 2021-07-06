#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>EvtGenSimRec.root</input>
  <output>SVDValidationTTree.root</output>
  <output>SVDValidationTTreeStrip.root</output>
  <output>SVDValidationTTreeSpacePoint.root</output>
  <output>SVDValidationTTreeSimhit.root</output>
  <output>SVDValidationTTreeRecoTrack.root</output>
  <output>SVDValidationTTreeRecoDigit.root</output>
  <output>SVDValidationTTreeTrueHit.root</output>
  <output>SVDValidationTTreeCluster.root</output>
  <contact>G. Caria, gcaria@student.unimelb.edu.au</contact>
  <description>This is the SVD validation steering file.</description>
</header>
"""

import basf2 as b2
from basf2 import conditions as b2c
# Individual validation packages
from SVDValidationTTree import SVDValidationTTree
from SVDValidationTTreeStrip import SVDValidationTTreeStrip
from SVDValidationTTreeSimhit import SVDValidationTTreeSimhit
from SVDValidationTTreeSpacePoint import SVDValidationTTreeSpacePoint
from SVDValidationTTreeRecoTrack import SVDValidationTTreeRecoTrack
from SVDValidationTTreeRecoDigit import SVDValidationTTreeRecoDigit
from SVDValidationTTreeTrueHit import SVDValidationTTreeTrueHit
from SVDValidationTTreeCluster import SVDValidationTTreeCluster

b2.set_random_seed(12345)

b2c.prepend_globaltag("svd_onlySVDinGeoConfiguration")

main = b2.create_path()

# Using 1000 EvtGen events already simulated and reconstructed
input = b2.register_module('RootInput')
input.param('inputFileName', '../EvtGenSimRec.root')
main.add_module(input)

# Gearbox and Geometry modules need to be registered anyway

# Load parameters
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Create geometry
main.add_module("Geometry")

# Show progress of processing
progress = b2.register_module('Progress')
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
svdvalidationtrack = SVDValidationTTreeRecoTrack()
main.add_module(svdvalidationtrack)
svdvalidationdigit = SVDValidationTTreeRecoDigit()
main.add_module(svdvalidationdigit)
svdvalidationtruehit = SVDValidationTTreeTrueHit()
main.add_module(svdvalidationtruehit)
svdvalidationcluster = SVDValidationTTreeCluster()
main.add_module(svdvalidationcluster)


b2.process(main)

# Print call statistics
print(b2.statistics)
