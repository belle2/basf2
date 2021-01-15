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

b2.logging.log_level = b2.LogLevel.ERROR
b2.set_debug_level(200)
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
main = b2.create_path()
main.add_module('EventInfoSetter',
                evtNumList=[1],
                runList=[1])
main.add_module('Gearbox')
main.add_module('Geometry',
                components=['CDC'])

b2.process(main)

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
