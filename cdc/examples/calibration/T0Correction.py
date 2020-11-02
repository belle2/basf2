import re
import datetime
import basf2 as b2
from ROOT import Belle2
from ROOT import gSystem
gSystem.Load('libcdc')

b2.reset_database()
b2.use_database_chain()
b2.use_central_database("Calibration_Offline_Development", b2.LogLevel.INFO)

d = datetime.datetime.today()
print(d.strftime('This Calculution is done at : %d-%m-%y %H:%M:%S\n'))

rootdir = None
param_dir = None
t0_file = None

with open('location') as file:
    lines = file.readlines()
    for line in lines:
        if re.match(r'dir_root', line):
            rootdir = line.split('"')[1]
        if re.match(r'dir_params', line):
            param_dir = line.split('"')[1]
        if re.match(r't0_file', line):
            t0_file = line.split('"')[1]
            t0file = param_dir + t0_file

inputfiles = rootdir + "/output_*"
print(t0file)
print(inputfiles)


main = b2.create_path()
main.add_module('EventInfoSetter',
                evtNumList=[1],
                runList=[1])
main.add_module('Gearbox')
main.add_module('CDCJobCntlParModifier',
                T0InputType=False,
                T0File=t0file)
main.add_module('Geometry',
                components=['CDC'])
b2.process(main)
Corr = Belle2.CDC.T0Correction()
Corr.inputFileNames(inputfiles)
Corr.outputFileName("t0.dat")
# Corr.InputFileNames("rootfile/output_cdc.*")
Corr.storeHisto(True)
Corr.setDebug(False)
Corr.setUseDB(False)
Corr.setMinimumNDF(10)
Corr.setMinimumPval(0.)
Corr.execute()
