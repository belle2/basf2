#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from generate_pxdclusters import add_generate_pxdclusters_bbbar
from generate_pxdclusters import GeneratePXDClusterConfig

import glob
import os
import shutil
import sys

# set_log_level(LogLevel.ERROR)

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Generate training data for computing cluster shape corrections")
    parser.add_argument('--outdir', default='tmp_bbbar', type=str, help='Directory to place all source files')
    parser.add_argument('--pixelkind', dest='pixelkind', default=4, type=int,
                        help='PixelKinds 0, 1, 2, 3 and 4 for z55, z60, z70, z85 and all pixel kinds')
    parser.add_argument('--nevents', dest='nevents', default=10, type=int, help='Number of events')
    parser.add_argument(
        '--bglocation',
        dest='bglocation',
        default='/home/benjamin/prerelease-01-00-00b-validation/samples',
        type=str,
        help='Location of bg overlay files')
    parser.add_argument('--turchetta', dest='turchetta', action="store_true",
                        help='Append Turchetta style estimator of hit position to source files')
    args = parser.parse_args()

    # Background overlay files
    bg = glob.glob(args.bglocation + '/*.root')
    if len(bg) == 0:
        print('No BG overlay files found in ', location)
        sys.exit()

    # Remove old source files, if needed
    if os.path.isdir(os.getcwd() + '/' + args.outdir):
        shutil.rmtree(os.getcwd() + '/' + args.outdir)

    # Ceeate new folder for source files
    os.mkdir(os.getcwd() + '/' + args.outdir)

    # Now lets configure the simulation of pxd clusters from command line options
    config = GeneratePXDClusterConfig()
    config.variables['Outdir'] = args.outdir
    config.variables['PixelKind'] = args.pixelkind
    config.variables['Turchetta'] = args.turchetta
    config.variables['nEvents'] = args.nevents

    # Now let's create a path to simulate our events.
    main = create_path()
    add_generate_pxdclusters_bbbar(main, config, bkgfiles=bg, bkgOverlay=True, use_default_pxd=True, min_mom=0.02, max_mom=6.0)

    main.add_module('RootOutput', outputFileName='output.root')

    # Process events
    process(main)

    # Print call statistics
    print(statistics)
