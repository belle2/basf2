#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import ROOT
import datetime
import os
import subprocess
from basf2 import *
from ROOT import Belle2
from tracking import add_cdc_cr_track_finding
from caf.framework import Calibration, CAF
from caf import backends
from cdc.cr import *

# reset_database()
use_local_database("/home/belle/muchida/basf2/release/cdc/examples/caf/localDB/database.txt")
data_period = '201702'
# trigger size, for create trigger Image cut(width(rphi plane), length(z direction)
triggerSize = [10, 100]

# trigger position.
triggerPos = [0., -1.5, 20]

# Normal trigger direction
normTriggerPlanDirection = [0, 1, 0]

# correct time of propagation of light in scitilator,
# if true, input correctly PMT position and prop. speed
correctToP = True

# Position of PMT that its signal is used for timing
readoutPos = [0., -1.5, -30]

# popagation speed of light in scintilator, =12,99 for 1m length scintillartor
propSpeedOfLight = 13.0  # cm/ns

probcut = 0.001

# The backend creates a file with the input data file paths inside it and places it in the job's working directory.
# This function reads the file automatically to gte a list of input file paths
from caf.backends import get_input_data

input_data = get_input_data()
output_file_name = 'CollectorOutput.root'

print("input : ", input_data)
print("output: ", output_file_name)

# Compose basf2 module path #
#############################
main_path = basf2.create_path()
logging.log_level = LogLevel.ERROR

# Master module: RootInput
main_path.add_module('RootInput',
                     inputFileNames=input_data)

# main_path.add_module('RootInput',
#                     inputFileNames=input_data,
#                     entrySequences=["0:1000"])

main_path.add_module('HistoManager', histoFileName=output_file_name)
main_path.add_module('Gearbox')
main_path.add_module('Geometry',
                     components=['CDC'])
# Progress module
main_path.add_module('Progress')

# track finding (Legendre finder)
add_cdc_cr_track_finding(main_path, trigger_point=triggerPos, merge_tracks=False)

main_path.add_module("SetupGenfitExtrapolation")
main_path.add_module("PlaneTriggerTrackTimeEstimator",
                     pdgCodeToUseForEstimation=13,
                     triggerPlanePosition=triggerPos,
                     triggerPlaneDirection=normTriggerPlanDirection,
                     useFittedInformation=False)

main_path.add_module("DAFRecoFitter",
                     probCut=0.00001,
                     pdgCodesToUseForFitting=13
                     )

main_path.add_module("PlaneTriggerTrackTimeEstimator",
                     pdgCodeToUseForEstimation=13,
                     triggerPlanePosition=triggerPos,
                     triggerPlaneDirection=normTriggerPlanDirection,
                     useFittedInformation=True,
                     useReadoutPosition=correctToP,
                     readoutPosition=readoutPos,
                     readoutPositionPropagationSpeed=propSpeedOfLight
                     )

main_path.add_module("DAFRecoFitter", logLevel=LogLevel.ERROR,
                     probCut=probcut,
                     pdgCodesToUseForFitting=13
                     )
main_path.add_module('TrackCreator',
                     pdgCodes=[13],
                     useClosestHitToIP=True
                     )
main_path.add_module('CDCCalibrationCollector',
                     granularity="all",
                     calExpectedDriftTime=True
                     )

basf2.print_path(main_path)
basf2.process(main_path)
print(statistics)
