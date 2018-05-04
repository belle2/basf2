#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import glob
import sys
import os
import shutil
from generate_pxdclusters import PrintPXDClusterShapesModule

set_log_level(LogLevel.ERROR)

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Generate training data for computing cluster shape corrections")
    parser.add_argument('--outdir', default='tmp_bbbar', type=str, help='Directory to place all source files')
    args = parser.parse_args()

    # Remove old source files, if needed
    if os.path.isdir(os.getcwd() + '/' + args.outdir):
        shutil.rmtree(os.getcwd() + '/' + args.outdir)

    # Ceeate new folder for source files
    os.mkdir(os.getcwd() + '/' + args.outdir)

    # Create path
    main = create_path()

    input = register_module('RootInput')
    # input.param('branchNames', ['PXDSimHits', 'PXDTrueHits', 'PXDTrueHitsToPXDSimHits', 'BeamBackHits'])
    main.add_module(input)

    # Gearbox: access to database (xml files)
    gearbox = register_module('Gearbox')
    main.add_module(gearbox)

    # Geometry
    geometry = register_module('Geometry')
    main.add_module(geometry)

    # PXD digitizer (no data reduction!)
    pxd_digitizer = register_module('PXDDigitizer')
    main.add_module(pxd_digitizer)

    pxd_clusterizer = register_module('PXDClusterizer')
    main.add_module(pxd_clusterizer)

    main.add_module(PrintPXDClusterShapesModule(outdir=args.outdir, pixelkind=4, turchetta=True, min_mom=0.0000, max_mom=10))

    # Show progress of processing
    progress = register_module('Progress')
    main.add_module(progress)

    # Process events
    process(main)

    # Print call statistics
    print(statistics)
