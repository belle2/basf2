#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import math
from generate_pxdclusters import add_generate_pxdclusters
from generate_pxdclusters import GeneratePXDClusterConfig
import os
import shutil

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Generate training data for computing cluster shape corrections")
    parser.add_argument('--outdir', default='tmp', type=str, help='Directory to place all source files')
    parser.add_argument('--magnet_on', dest='magnet_on', action="store_true", help='Turn on magnetic field')
    parser.add_argument('--nevents', dest='nevents', default=100000, type=int, help='Number of clusters per step')
    parser.add_argument('--pixelkind', dest='pixelkind', default=0, type=int,
                        help='PixelKinds 0, 1, 2, 3 for z55, z60, z70, z85 pixels')
    parser.add_argument('--turchetta', dest='turchetta', action="store_true",
                        help='Append Turchetta style estimator of hit position to source files')
    parser.add_argument('--theta_min', dest='theta_min', default=0, type=float, help='Minimum theta angle for particle gun')
    parser.add_argument('--theta_max', dest='theta_max', default=180, type=float, help='Maximum theta angle for particle gun')
    parser.add_argument('--phi_min', dest='phi_min', default=0, type=float, help='Minimum phi angle for particle gun')
    parser.add_argument('--phi_max', dest='phi_max', default=180, type=float, help='Maximum phi angle for particle gun')
    args = parser.parse_args()

    # Remove old source files, if present
    if os.path.isdir(os.getcwd() + '/' + args.outdir):
        shutil.rmtree(os.getcwd() + '/' + args.outdir)

    # Create nice clean folder for sources
    os.mkdir(os.getcwd() + '/' + args.outdir)

    # Now lets configure the simulation of pxd clusters from command line options
    config = GeneratePXDClusterConfig()
    config.variables['Outdir'] = args.outdir
    config.variables['Magnet-Off'] = not args.magnet_on
    config.variables['nEvents'] = args.nevents
    config.variables['ThetaParams'] = [args.theta_min, args.theta_max]
    config.variables['PhiParams'] = [args.phi_min, args.phi_max]
    config.variables['PixelKind'] = args.pixelkind
    config.variables['Turchetta'] = args.turchetta
    # config.variables['Momenta'] = [4.0]
    # config.variables['pdgCodes'] = [-11]

    # Now let's create a path to simulate our events.
    main = create_path()
    add_generate_pxdclusters(main, config, use_default_pxd=True)

    process(main)
    print(statistics)
