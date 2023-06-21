#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# Simple steering file to demonstrate how to run testbeam
#
# Usage: python3 testbeam_example.py  -forceCoG (-n 100000 -angle 0)
#############################################################


import basf2 as b2
from VTXOutputDumper import VTXOutputDumper

import argparse
parser = argparse.ArgumentParser(description="Perform runs of test beam")
parser.add_argument('-n', default=10000, type=int, help='Number of events to generate')
parser.add_argument('-angle', default=0.0, type=float, help='Angle (deg) for the particleGun (in phi)')
parser.add_argument('-forceCoG', default=False, action='store_true', help="Use only(!) center of gravity based position finding")
parser.add_argument('-outputfile', default='', type=str, help='Path of output file')
parser.add_argument('-eToADU', default=50.0, type=float, help='Conversion from collected charge in e- to ADU code')
parser.add_argument('-height', default=0.030, type=float, help='Depleted height in mm')
parser.add_argument('-thr', default=150.0, type=float, help='Hit threshold in e-')
parser.add_argument('-cloudSize', default=0.000085, type=float, help='Diffusion coefficient')
parser.add_argument('-cce', default=1.0, type=float, help='Charge collection efficiency, should be 1.0 for non HV')
parser.add_argument('-tb_a', default=0.118, type=float, help='ToT calibration coefficient a')
parser.add_argument('-tb_b', default=1.3, type=float, help='ToT calibration coefficient b')
parser.add_argument('-tb_c', default=1.4e2, type=float, help='ToT calibration coefficient c')
parser.add_argument('-tb_t', default=4e1, type=float, help='ToT calibration coefficient t')
args = parser.parse_args()


# Number of events to generate
num_events = args.n

# Angle for the particleGun (in phi)
angle = args.angle

# Output file name
if args.outputfile == '':
    outputfile = f"testbeam_{angle}deg.root"
else:
    outputfile = args.outputfile

# Sanitize inputs: conversion factor
if args.eToADU <= 5:
    args.eToADU = 5

# Sanitize inputs: thickness of depleted Si in mm
if args.height <= 0.015:
    args.height = 0.015

# Sanitize inputs: threshold in e-
if args.thr <= 100:
    args.thr = 100

# create path
main = b2.create_path()

main.add_module("EventInfoSetter", evtNumList=num_events)

gearbox = main.add_module("Gearbox")
gearbox.param({
    "overridePrefix": "//DetectorComponent[@name='VTX-CMOS-testbeam-2022-07-01']/Content/Components/Sensor/Active/",
    "override": [
        ("height", str(args.height), "mm"),
        ("BinaryHitThreshold", str(args.thr), "ENC"),
        ("ElectronToADU", str(args.eToADU), "ENC"),
        ("CloudSize", str(args.cloudSize), "mm"),
    ],
})

main.add_module('ParticleGun', pdgCodes=[11],
                momentumGeneration="fixed", momentumParams=[5.2],
                thetaGeneration="fixed", thetaParams=[90.],
                phiGeneration="fixed", phiParams=[angle],
                # normal distributed vertex generation
                vertexGeneration='normal',
                xVertexParams=[-0.02, 0.0],
                yVertexParams=[0.28, 0.6],
                zVertexParams=[0.95, 0.6]
                )

main.add_module('Gearbox')

excluded_parts = [
    'PXD',
    'SVD',
    'CDC',
    'ECL',
    'ARICH',
    'TOP',
    'KLM',
    'COIL',
    'STR',
    'ServiceGapsMaterial',
    'BeamPipe',
    'Cryostat',
    'FarBeamLine',
    'HeavyMetalShield',
    'VXDService',
    'MagneticField'
]

main.add_module("Geometry",
                excludedComponents=excluded_parts,
                additionalComponents=["VTX-CMOS-testbeam-2022-07-01"],
                useDB=False)

# G4 simulation
main.add_module('FullSim')

# Digitizer
main.add_module('VTXDigitizer', UseToTCalibration=True,
                ToTCoefficient_a=args.tb_a,
                ToTCoefficient_b=args.tb_b,
                ToTCoefficient_c=args.tb_c,
                ToTCoefficient_t=args.tb_t,
                ChargeCollectionEfficiency=args.cce)

# Activate cluster shape correction
if not args.forceCoG:
    main.add_module("ActivateVTXClusterPositionEstimator")

# Clusturizer
main.add_module('VTXClusterizer')

# Make the tree
output = VTXOutputDumper(outputfile)
main.add_module(output)

# process events and print call statistics
main.add_module('Progress')
b2.process(main)
print(b2.statistics)
