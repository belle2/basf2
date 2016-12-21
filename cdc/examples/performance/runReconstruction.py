import basf2
from basf2 import *
import ROOT
import datetime
import os
import os.path
import argparse
from tracking import add_cdc_cr_track_finding
from ROOT import Belle2

reset_database()
# use_local_database("cdc_crt/database.txt", "cdc_crt", False)
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_central_database("cdc_cr_test1", LogLevel.WARNING)
set_experiment_name(0, "cdc_crt_e000000")

run_range = {'201607': [787, 833],
             '201608a': [885, 896],
             '201608b': [917, 924],
             '201609': [966, 973]
             }

triggerPosition = {'201607': [0.3744, 0.0, -1.284],
                   '201608a': [-1.87, -1.25, 18.7],
                   '201608b': [-1.87, -1.25, 11.0],
                   '201609': [0, 0, 11.0]}

triggerPlaneDirection = {'201607': [1, -1, 0],
                         '201608a': [0, 1, 0],
                         '201608b': [0, 1, 0],
                         '201609': [0, 1, 0]
                         }

pmtPosition = {'201607': [0, 0, 0],
               '201608a': [-1.87, 0, -25.0],
               '201608b': [-1.87, 0, -42.0],
               '201609': [0, 0, -42.0]
               }

globalPhiRotation = 1.875


def main(input, output):
    main_path = basf2.create_path()
    logging.log_level = LogLevel.INFO

    # Get run number.
    run_number = int((input.split('/')[-1]).split('.')[3])

    # Set the peiod of data taking.
    data_period = None
    for key in run_range:
        if run_range[key][0] <= run_number <= run_range[key][1]:
            data_period = key
            print("Data in the period " + key + " is analysed.")
            break

    if data_period is None:
        B2ERROR("No valid data period is specified.")

    if os.path.exists(data_period) is False:
        os.mkdir(data_period)
    output = "/".join([data_period, output])
    # RootInput
    main_path.add_module('RootInput',
                         inputFileNames=input)

    # gearbox & geometry needs to be registered any way
    main_path.add_module('Gearbox',
                         override=[
                             ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", str(globalPhiRotation), "deg")
                         ])
    #
    main_path.add_module('Geometry',
                         components=['CDC'])
    # geometry = register_module('Geometry')
    # geometry.param('useDB',True)
    # main_path.add_module(geometry)
    # Progress module
    main_path.add_module('Progress')

    # Add cdc track finder
    add_cdc_cr_track_finding(main_path)

    main_path.add_module("SetupGenfitExtrapolation")
    main_path.add_module("PlaneTriggerTrackTimeEstimator",
                         pdgCodeToUseForEstimation=13,
                         triggerPlanePosition=triggerPosition[data_period],
                         triggerPlaneDirection=triggerPlaneDirection[data_period],
                         useFittedInformation=False)

    main_path.add_module("DAFRecoFitter",
                         probCut=0.00001,
                         pdgCodesToUseForFitting=13,
                         )

    main_path.add_module("PlaneTriggerTrackTimeEstimator",
                         pdgCodeToUseForEstimation=13,
                         triggerPlanePosition=triggerPosition[data_period],
                         triggerPlaneDirection=triggerPlaneDirection[data_period],
                         useFittedInformation=True,
                         useReadoutPosition=False if data_period == "201607" else True,
                         readoutPosition=pmtPosition[data_period],
                         readoutPositionPropagationSpeed=12.9925
                         )

    main_path.add_module("DAFRecoFitter",
                         #                     probCut=0.00001,
                         pdgCodesToUseForFitting=13,
                         )
    main_path.add_module('TrackCreator',
                         defaultPDGCode=13,
                         useClosestHitToIP=True
                         )

    main_path.add_module('CDCCosmicAnalysis',
                         Output=output)

    basf2.print_path(main_path)
    basf2.process(main_path)

    print(basf2.statistics)


if __name__ == "__main__":
    # Make the parameters accessible form the outside.

    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Input file to be processed (unpacked CDC data).')
    parser.add_argument('output', help='Output file you want to store the results.')
    args = parser.parse_args()

    date = datetime.datetime.today()
    print(date.strftime('Start at : %d-%m-%y %H:%M:%S\n'))
    main(args.input, args.output)
    date = datetime.datetime.today()
    print(date.strftime('End at : %y-%m-%d %H:%M:%S\n'))
