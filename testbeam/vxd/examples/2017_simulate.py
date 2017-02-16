#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from testbeam.utils import *

import argparse
parser = argparse.ArgumentParser(description="Simulation for DESY VXD Testbeam 2017")
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
parser.add_argument('--field', dest='field', action='store', default=None, type=float,
                    help='Magnetuc field in Tesla. If set and magnet ON, the provided value is used as constant field')
parser.add_argument('--momentum', dest='momentum', action='store', default=5., type=float,
                    help='Nominal momentum of particles. Default = 5 GeV/c')
parser.add_argument('--display', dest='display', action='store_const', const=True, default=False, help='Show Event Display window')
args = parser.parse_args()


setup_database(args.local_db, args.global_tag)

main = create_path()

main.add_module('EventInfoSetter')

add_geometry(main, magnet=not args.magnet_off, field_override=args.field, target=None)
add_simulation(main, momentum=args.momentum, positrons=False, telescopes=False)

if args.display:
    main.add_module('Display', fullGeometry=True)

main.add_module('Progress')
main.add_module('RootOutput')

process(main)

print(statistics)
