import basf2
import ROOT
import datetime
import os
import re
from basf2 import *
from ROOT import Belle2
from ROOT.Belle2 import FileSystem
from ROOT import gSystem
gSystem.Load('libcdc')
# gSystem.SetBatch(1);

reset_database()
use_database_chain()
use_central_database("Calibration_Offline_Development", LogLevel.INFO)

logging.log_level = LogLevel.ERROR
set_debug_level(200)
d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))


rootdir = None
param_dir = None
tw_file = None

with open('location') as file:
    lines = file.readlines()
    for line in lines:
        if re.match(r'dir_root', line):
            rootdir = line.split('"')[1]
        if re.match(r'dir_params', line):
            param_dir = line.split('"')[1]
        if re.match(r'tw_file', line):
            tw_file = line.split('"')[1]
            twfile = param_dir + tw_file
print(twfile)
main = create_path()
main.add_module('EventInfoSetter',
                evtNumList=[1],
                runList=[1])
main.add_module('Gearbox')
main.add_module('Geometry',
                components=['CDC'])

basf2.process(main)

tw = Belle2.CDC.TimeWalkCalibration()
tw.InputFileNames(rootdir + "/output_*")
tw.InputTWFileName(twfile)
tw.OutputTWFileName("tw.dat")
tw.setStoreHisto(True)
tw.setDebug(True)
tw.setUseDB(False)
tw.setMinimumNDF(10)
tw.setMinimumPval(0.001)
# tw.setBinWidth(0.025)
tw.execute()
