#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Simple basf2 steering file that creates mc file with PXDSimHits and
# EventMetaData needed for PXD gain calibration.
#
# Note that this script should only be used for runs in phase2 geometry
# with beams and magnetic field.
#
# Note that only SimHits from bg will be produced. A path to the mixer
# files for phase 2 must be specified. Only use for  runs with beam
# where most clusters are from bg.
#
# The command below will create reference mc file for gain calibration
# for run=3360 in exp=3 being part of the phase2 campaign.
#
# basf2 create_mcrun.py -- --tag='Calibration_Offline_Development'  --expNo=3 --runNo=3360 --setNo=0
#
# author: benjamin.schwenker@phys.uni-goettingen.de


import glob
import basf2 as b2

import argparse
parser = argparse.ArgumentParser(
    description="Create SimHits for a run with user specified ExpRun")
parser.add_argument(
    '--tag',
    default='Calibration_Offline_Development',
    type=str,
    help='Set name of GT')
parser.add_argument(
    '--expNo',
    default=3,
    type=int,
    help='Set experiment number')
parser.add_argument('--runNo', default=3360, type=int, help='Set run number')
parser.add_argument(
    '--setNo',
    default=0,
    type=int,
    help='setnumber for bg simulation')
parser.add_argument(
    '--bg',
    default='/group/belle2/BGFile/OfficialBKG/15thCampaign/phase2',
    type=str,
    help='Path to mixer sets')
parser.add_argument(
    '--scaleFactor',
    default=1.0,
    type=float,
    help='Scale factor for mixer')
args = parser.parse_args()

bg = glob.glob(args.bg + '/set' + str(args.setNo) + '/*.root')


b2.reset_database()
b2.use_central_database(args.tag)

main = b2.create_path()
main.add_module(
    "EventInfoSetter", expList=[
        args.expNo], runList=[
            args.runNo], evtNumList=[1000])
main.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
main.add_module("Geometry", useDB=False)
bkgmixer = b2.register_module('BeamBkgMixer')
bkgmixer.param('backgroundFiles', bg)
bkgmixer.param('overallScaleFactor', args.scaleFactor)
main.add_module(bkgmixer)
output = main.add_module('RootOutput')
output.param(
    'outputFileName',
    'beam.{:0>4}.{:0>5}.HLT2.f{:0>5}.root'.format(
        args.expNo,
        args.runNo,
        args.setNo))
output.param('branchNames', ['PXDSimHits', 'EventMetaData'])
main.add_module("Progress")

b2.process(main)
print(b2.statistics)
