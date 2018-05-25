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
logging.log_level = LogLevel.INFO
set_debug_level(200)

d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))

rootdir = None
param_dir = None
xt_file = None

with open('location') as file:
    lines = file.readlines()
    for line in lines:
        if re.match(r'dir_root', line):
            rootdir = line.split('"')[1]
        if re.match(r'dir_params', line):
            param_dir = line.split('"')[1]
        if re.match(r'xt_file', line):
            xt_file = line.split('"')[1]

xtfile = param_dir + xt_file
rootfile = rootdir + "/output_*"

main = create_path()
main.add_module('EventInfoSetter',
                evtNumList=[1],
                runList=[1])
basf2.process(main)

fit = Belle2.CDC.XTCalibration()
fit.inputFileNames(rootfile)
fit.profileFileNames("xt_profile.dat")
fit.useProfileFromInputXT(False)
fit.setXTFileName(xtfile)
# fit.setUseSliceFit(False);
fit.setStoreHisto(True)
fit.setDebug(False)
fit.setUseDB(False)
fit.setMinimumNDF(30)
fit.setMinimumPval(0.0001)
fit.execute()
