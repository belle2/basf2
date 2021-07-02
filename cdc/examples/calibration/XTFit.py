##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
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

main = b2.create_path()
main.add_module('EventInfoSetter',
                evtNumList=[1],
                runList=[1])
b2.process(main)

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
