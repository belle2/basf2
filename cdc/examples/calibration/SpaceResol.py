import datetime
import re
import basf2 as b2
from ROOT import Belle2
from ROOT import gSystem
gSystem.Load('libcdc')
# gSystem.SetBatch(1);

b2.reset_database()
b2.use_database_chain()
b2.use_central_database("Calibration_Offline_Development", b2.LogLevel.INFO)

b2.logging.log_level = b2.LogLevel.INFO
b2.set_debug_level(200)

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

main = b2.create_path()
main.add_module('EventInfoSetter',
                evtNumList=[1],
                runList=[1])
main.add_module('Gearbox')
main.add_module('Geometry',
                components=['CDC'])

b2.process(main)

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
