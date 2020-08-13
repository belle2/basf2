#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from tracking import *
from svd import *
from rawdata import *
from ROOT import Belle2
from reconstruction import add_reconstruction
import os.path
import sys

fileIN = sys.argv[1]
dirOUT = sys.argv[2]

main = create_path()
main.add_module('RootInput', inputFileName=str(fileIN))

gearbox = register_module('Gearbox')
geomfile = '/geometry/Beast2_phase2.xml'
if geomfile != 'None':
    gearbox.param('fileName', geomfile)
main.add_module(gearbox)
geometry = register_module('Geometry', components=['SVD'])
main.add_module(geometry, useDB=False)

main.add_module('SVDChargeSharingAnalysis', outputDirName=str(dirOUT), outputRootFileName='test.root',
                useTrackInfo=True, is2017TBanalysis=False)
main.add_module('Progress')
print_path(main)
process(main)
print(statistics)
