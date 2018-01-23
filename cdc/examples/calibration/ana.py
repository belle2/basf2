import basf2
import ROOT
import datetime
import os
import subprocess
from basf2 import *
from ROOT import Belle2
from tracking import add_cr_tracking_reconstruction

# reset_database()
# use_local_database("centraldb/dbcache.txt")

d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))
# reunpack the data
reUnpack = True
# trigger size, for create trigger Image cut(width(rphi plane), length(z direction)
triggerSize = [50, 200]

# trigger position.
triggerPos = [0., 0., 0.]

# Normal trigger direction
normTriggerPlanDirection = [0, 1, 0]


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
prefix = None
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
        if re.match(r'prefix', line):
            prefix = line.split('"')[1]
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
# getname
names = name.split(".")

inputfilename = datadir + "r{0:05}/sub00/".format(int(names[2])) + name
outputfilename = rootdir + '/output_' + name
outputfilename2 = rootdir + '/twotracks_' + name
logfilename = rootdir + '/run_' + name

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
if reUnpack:
    main_path.add_module('CDCUnpacker',
                         # Enable/Disable to store the CDCRawHit Object.
                         enableStoreCDCRawHit=True,
                         enableDatabase=False,
                         xmlMapFileName="cdc/data/ch_map_201702.dat",
                         # Enable/Disable print out the ADC/TDC data to the terminal.
                         enablePrintOut=False,
                         enable2ndHit=False,
                         boardIDTrig=7,
                         channelTrig=1,
                         subtractTrigTiming=False
                         )

# gearbox & geometry needs to be registered any way
main_path.add_module('Gearbox')
main_path.add_module('CDCJobCntlParModifier',
                     MapperGeometry=True,
                     MapperPhiAngle=43.3,
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
main_path.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'ARICH', 'BeamPipe', 'HeavyMetalShield'])
# Progress module
main_path.add_module('Progress')
main_path.add_module('SetupGenfitExtrapolation')
add_cr_tracking_reconstruction(path=main_path, prune_tracks=False,
                               skip_geometry_adding=False,
                               event_time_extraction=True,
                               data_taking_period="gcr2017",
                               top_in_counter=False,
                               merge_tracks=False,
                               use_second_cdc_hits=False)
'''
#for analysis, if you want to check performance please enable this part
main_path.add_module('CDCCosmicAnalysis',
                     Output = outputfilename2,
                     EventT0Extraction = True,
                     noBFit = False
                     )
'''
main_path.add_module('CDCCRTest', logLevel=LogLevel.ERROR,
                     RecoTracksColName='RecoTracks',
                     histogramDirectoryName='trackfit',
                     MinimumPt=0.1,
                     noBFit=False,
                     EventT0Extraction=True,
                     plotResidual=False,
                     calExpectedDriftTime=True,
                     TriggerPos=triggerPos,
                     NormTriggerPlaneDirection=normTriggerPlanDirection,
                     TriggerSize=triggerSize,
                     EstimateResultForUnFittedLayer=False,
                     StoreHitDistribution=False,
                     StoreTrackParams=False,
                     SmallerOutput=True
                     )

main_path.add_module('HistoManager', histoFileName=outputfilename)
# main_path.add_module('RootOutput',
#                     outputFileName = datadir + "evtT0." +prefix + name+'.root',
#                     branchNames = ['CDCHits','EventT0'])
basf2.print_path(main_path)
basf2.process(main_path)

d = datetime.datetime.today()
print(statistics)
print(d.strftime('Finish at : %y-%m-%d %H:%M:%S\n'))
