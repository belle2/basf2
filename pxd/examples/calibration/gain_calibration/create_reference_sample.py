#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2

from ROOT import gROOT

import os
import shutil

from generate_pxdclusters import PrintSimplePXDClusterShapesModule

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Generate training data for computing cluster shape corrections")
    parser.add_argument('--outdir', default='tmp', type=str, help='Directory to place all source files')
    parser.add_argument('--pixelkind', dest='pixelkind', default=0, type=int,
                        help='PixelKinds 0, 1, 2, 3 for z55, z60, z70, z85 pixels')
    args = parser.parse_args()

    # Remove old source files, if present
    if os.path.isdir(os.getcwd() + '/' + args.outdir):
        shutil.rmtree(os.getcwd() + '/' + args.outdir)

    # Create nice clean folder for sources
    os.mkdir(os.getcwd() + '/' + args.outdir)

    rootinput = register_module('RootInput')
    gearbox = register_module('Gearbox')
    geometry = register_module('Geometry')
    progress = register_module('Progress')
    pxdhitsorter = register_module('PXDRawHitSorter')
    pxdclusterizer = register_module('PXDClusterizer')
    # output = register_module('RootOutput')
    clusterprinter = PrintSimplePXDClusterShapesModule(outdir=args.outdir, pixelkind=args.pixelkind)
    # Select Beast2_phase2 geometry
    gearbox.param('fileName', 'geometry/Beast2_phase2.xml')

    main = create_path()
    main.add_module(rootinput)
    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module('PXDUnpacker')
    main.add_module("ActivatePXDPixelMasker")
    main.add_module(pxdhitsorter)
    main.add_module(pxdclusterizer)
    main.add_module(clusterprinter)
    main.add_module(progress)

    # Process events
    process(main)

    # Print call statistics
    print(statistics)
