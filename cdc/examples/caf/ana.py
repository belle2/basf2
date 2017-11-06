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

# reset_database()
# use_local_database("centraldb/dbcache.txt")

d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))


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

argvs = sys.argv
name = argvs[1]
argc = len(argvs)
probcut = 0
if argc == 3:
    probcut = argvs[2]
elif argc == 2:
    probcut = 0.001

probcut = float(probcut)
# print(probcut)
import re

rootdir = None
datadir = None
param_dir = None
t0_file = None
tw_file = None
xt_file = None
sigma_file = None

with open('location') as file:
    lines = file.readlines()
    for line in lines:
        if re.match(r'dir_root', line):
            rootdir = line.split('"')[1]
        if re.match(r'dir_data', line):
            datadir = line.split('"')[1]
        if re.match(r'dir_params', line):
            param_dir = line.split('"')[1]
        if re.match(r't0_file', line):
            t0_file = line.split('"')[1]
        if re.match(r'tw_file', line):
            tw_file = line.split('"')[1]
        if re.match(r'xt_file', line):
            xt_file = line.split('"')[1]
        if re.match(r'sigma_file', line):
            sigma_file = line.split('"')[1]

inputfilename = datadir + 'cr.' + name + '.root'
outputfilename = rootdir + '/output_' + name + '.root'
outputfilename2 = rootdir + '/CAF_' + name + '.root'
logfilename = rootdir + '/run_' + name + '.log'

# param_dir = '201607/'

print("input : ", inputfilename)
print("output: ", outputfilename)
print("log   : ", logfilename)

# Compose basf2 module path #
#############################
main_path = basf2.create_path()
logging.log_level = LogLevel.ERROR

# Master module: RootInput
main_path.add_module('RootInput',
                     inputFileNames=inputfilename)

# gearbox & geometry needs to be registered any way
main_path.add_module('Gearbox')
main_path.add_module('CDCJobCntlParModifier',
                     T0InputType=False,
                     T0File=param_dir + t0_file,
                     XtInputType=False,
                     XtFile=param_dir + xt_file,
                     SigmaInputType=False,
                     SigmaFile=param_dir + sigma_file,
                     TimeWalkInputType=False,
                     TimeWalkFile=param_dir + tw_file,
                     ChannelMapInputType=False,
                     ChannelMapFile="ch_map_201702.dat",
                     AlignmentInputType=False,
                     AlignmentFile=param_dir + "align_201702.dat"
                     )
main_path.add_module('Geometry',
                     components=['CDC'])
# Progress module
main_path.add_module('Progress')

# track finding (Legendre finder)
add_cdc_cr_track_finding(main_path, trigger_point=triggerPos, merge_tracks=False)

# main_path.add_module("CDCRecoTrackFilter",ExcludeSLayer=[0,1,2])

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
main_path.add_module('CDCCRTest', logLevel=LogLevel.ERROR,
                     RecoTracksColName='RecoTracks',
                     histogramDirectoryName='trackfit',
                     plotResidual=False,
                     calExpectedDriftTime=True,
                     TriggerPos=triggerPos,
                     NormTriggerPlaneDirection=normTriggerPlanDirection,
                     TriggerSize=triggerSize,
                     EstimateResultForUnFittedLayer=False,
                     StoreHitDistribution=False,
                     SmallerOutput=True,
                     )
main_path.add_module('HistoManager', histoFileName=outputfilename)

collector = basf2.register_module('CDCT0CalibrationCollector',
                                  granularity="all",
                                  NDFCut=5
                                  )
# collector.set_name('T0CalibCollector')
main_path.add_module(collector)
main_path.add_module('RootOutput', branchNames=['EventMetaData'], outputFileName=outputfilename2)


basf2.print_path(main_path)
basf2.process(main_path)

d = datetime.datetime.today()
print(statistics)
print(d.strftime('Finish at : %y-%m-%d %H:%M:%S\n'))
