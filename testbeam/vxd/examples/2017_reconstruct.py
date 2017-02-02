#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from testbeam.utils import *

import argparse
parser = argparse.ArgumentParser(description="Reconstruction for DESY VXD Testbeam 2016")
parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--global-tag',
    dest='global_tag',
    action='store',
    default=None,
    type=str,
    help='Global tag to use at central DB in PNNL')
parser.add_argument(
    '--magnet-off',
    dest='magnet_off',
    action='store_const',
    const=True,
    default=False,
    help='Turn off magnetic field')
parser.add_argument('--momentum', dest='momentum', action='store', default=6., type=float,
                    help='Nominal momentum of particles (if magnet is off). Default = 6 GeV/c')
parser.add_argument('--field', dest='field', action='store', default=1., type=float,
                    help='Magnetuc field in Tesla. If different from 1. and magnet ON, the provided value is used')
parser.add_argument('--svd-only', dest='svd_only', action='store_const', const=True,
                    default=False, help='Use only SVD sector maps in VXDTF track finder')
parser.add_argument('--unpacking', dest='unpacking', action='store_const', const=True,
                    default=False, help='Add PXD and SVD unpacking modules to the path')
parser.add_argument('--dqm', dest='dqm', action='store_const', const=True, default=False, help='Produce DQM plots')
parser.add_argument('--display', dest='display', action='store_const', const=True, default=False, help='Show Event Display window')

parser.add_argument(
    '--tel-input',
    dest='tel_input',
    action='store',
    default=None,
    type=str,
    help='Location of telescope .raw file to merge')
args = parser.parse_args()


setup_database(args.local_db, args.global_tag)

main = create_path()

# Limit branches to only neccessary minimum (removes MC info if input is from simulation, HLT output)
branches = ['EventMetaData', 'RawFTSWs', 'RawSVDs', 'RawPXDs']
if not args.unpacking:
    branches = branches + ['PXDDigits', 'SVDDigits']
main.add_module('RootInput', branchNames=branches)

add_geometry(main, magnet=not args.magnet_off, field_override=args.field, target=None)

if args.unpacking:
    add_unpacking(main, svd_only=args.svd_only)

add_reconstruction(main, magnet=not args.magnet_off, svd_only=args.svd_only, telescopes=False, momentum=args.momentum)


def add_dqms(path, raw_dqm=True, svd_only=False):
    path.add_module('HistoManager')
    if not svd_only:
        if raw_dqm:
            path.add_module("PXDRawDQM")
        path.add_module("PXDDQMCorr")
        path.add_module('PXDDQM', histgramDirectoryName='pxddqm')

    path.add_module('SVDDQM3')

    """
    main.add_module("VXDTelDQMOffLine", SaveOtherHistos=1, CorrelationGranulation=0.5)

    if telescopes:
        main.add_module("TelDQM")
    """

    main.add_module('TrackfitDQM')

if args.dqm:
    add_dqms(main, raw_dqm=args.unpacking, svd_only=args.svd_only)

if args.display:
    main.add_module('Display', fullGeometry=True)

main.add_module('Progress')
main.add_module('RootOutput')

process(main)

print(statistics)
