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


# reset_database()
# use_local_database()
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
# use_local_database("cdc_crt/database.txt")

logging.log_level = LogLevel.INFO
set_debug_level(200)

d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))

# For file name
rootdir = None
param_dir = None
sigma_file = None
with open('location') as file:
    lines = file.readlines()
    for line in lines:
        if re.match(r'dir_root', line):
            rootdir = line.split('"')[1]
        if re.match(r'dir_params', line):
            param_dir = line.split('"')[1]
        if re.match(r'sigma_file', line):
            sigma_file = line.split('"')[1]
            sigmafile = param_dir + sigma_file
print(sigmafile)
rootfile = rootdir + "/output_*"
# rootfile="rootfile_finalMapper_newSigma/output_*"

main = create_path()
main.add_module('EventInfoSetter',
                evtNumList=[1],
                runList=[1])
main.add_module('Gearbox')
main.add_module('Geometry',
                components=['CDC'])

basf2.process(main)

fit = Belle2.CDC.SpaceResolutionCalibration()
fit.inputFileNames(rootfile)
fit.ProfileFileNames("sigma_profile.dat")
fit.useProfileFromInputSigma(False)
fit.setSigmaFileName(sigmafile)
fit.setStoreHisto(True)
fit.setDebug(False)
fit.setUseDB(False)
fit.setMinimumNDF(30)
fit.setBinWidth(0.05)
fit.setMinimumPval(0.001)
fit.execute()
