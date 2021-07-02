#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##############################################################################
# This steering file create a field map of the Belle II detector magnetic
# field and saves it as a set of 2D histograms in a root file
##############################################################################

import os
import math
from basf2 import Path, process
import subprocess

commit = subprocess.check_output(["git", "--git-dir=%s/.git" % os.environ["BELLE2_LOCAL_DIR"],
                                  "rev-parse", "--short", "HEAD"]).decode().strip()

path = Path()
path.add_module("EventInfoSetter")
path.add_module("CreateFieldMap", **{
    # Filename for the histograms
    'filename': 'FieldMap-%s.root' % commit,
    # type of the scan: along xy, zx or zy
    "type": "zx",
    # number of steps along the first coordinate, in the case of a zx scan this
    # would be z
    "nU": 1600,
    # start of the first coordinate in cm
    "minU": -350,
    # end of the first coordinate in cm
    "maxU": 450,
    # number of steps along the second coordinate, in the case of a zx scan
    # this would be x
    "nV": 1600,
    # start of the second coordinate in cm
    "minV": -400,
    # end of the second coordinate in cm
    "maxV": 400,
    # rotation the plane for the scan around the global z axis: a value of
    # pi/2 will convert a zx in a zy scan
    "phi": math.pi / 4,
    # offset of the plane with respect to the global origin
    "wOffset": 0,
    # number of steps in phi for zr scan
    "nPhi": 720,
    # if true here store all sampled points in a TTree
    "saveAllPoints": False,
})
process(path)
