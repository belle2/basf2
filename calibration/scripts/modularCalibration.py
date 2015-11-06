#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

import fnmatch

calibration_main = create_path()
input_path = create_path()
reprocessing_path = create_path()
collection_path = create_path()
calibration_path = create_path()

misalignment_scenario_name = None
magnet_off_run_list = []
magnet_off_exp_list = []
calibration_range = None
dst_storage_pattern = None


def getCommandLineOptions():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-nocal', '--no-calibrate', dest='nocalibrate', action='store_true', help='Do not perform calibration')
    parser.add_argument('-nocol', '--no-collect', dest='nocollect', action='store_true', help='Do not perform data collection')
    args = parser.parse_args()
    return args


class VXDMisalignment:

    def __init__(self, name):
        self.name = name
        self.misalignment = []

    def getName(self):
        return self.name

    def getMisalignment(self):
        return self.misalignment

    def misalignSensors(self, vxdid, param_id, distribution, value):
        self.misalignment.append(str(vxdid + ',' + str(param_id) + ',' + distribution + ',' + str(value)))

    def genSensorU(self, vxdid, value):
        self.misalignSensors(vxdid, 1, 'gaus', value)

    def genSensorV(self, vxdid, value):
        self.misalignSensors(vxdid, 2, 'gaus', value)

    def genSensorW(self, vxdid, value):
        self.misalignSensors(vxdid, 3, 'gaus', value)

    def genSensorAlpha(self, vxdid, value):
        self.misalignSensors(vxdid, 4, 'gaus', value)

    def genSensorBeta(self, vxdid, value):
        self.misalignSensors(vxdid, 5, 'gaus', value)

    def genSensorGamma(self, vxdid, value):
        self.misalignSensors(vxdid, 6, 'gaus', value)

    def setSensorU(self, vxdid, value):
        self.misalignSensors(vxdid, 1, 'fix', value)

    def setSensorV(self, vxdid, value):
        self.misalignSensors(vxdid, 2, 'fix', value)

    def setSensorW(self, vxdid, value):
        self.misalignSensors(vxdid, 3, 'fix', value)

    def setSensorAlpha(self, vxdid, value):
        self.misalignSensors(vxdid, 4, 'fix', value)

    def setSensorBeta(self, vxdid, value):
        self.misalignSensors(vxdid, 5, 'fix', value)

    def setSensorGamma(self, vxdid, value):
        self.misalignSensors(vxdid, 6, 'fix', value)

    def setSensorAll(self, vxdid, value):
        self.setSensorU(vxdid, value)
        self.setSensorV(vxdid, value)
        self.setSensorW(vxdid, value)
        self.setSensorAlpha(vxdid, value)
        self.setSensorBeta(vxdid, value)
        self.setSensorGamma(vxdid, value)


def generateMisalignment(vxdMisalignment, expLow=0, runLow=0, expHigh=-1, runHigh=-1):
    input_path.add_module(
        'MisalignmentGenerator',
        experimentLow=expLow,
        runLow=runLow,
        experimentHigh=expHigh,
        runHigh=runHigh,
        name=vxdMisalignment.getName(),
        data=vxdMisalignment.getMisalignment())


def granularityData():
    global calibration_range
    cal_on_exprun = [{'exp': calibration_range[2], 'run':calibration_range[3]}]
    return '{}.{}.{}.{}'.format(calibration_range[0], calibration_range[1], cal_on_exprun[0]['exp'], cal_on_exprun[0]['run'])
"""
def granularityRun(each=None, list=[], exclude=[]):
  global calibration_range
  return str(calibration_range[0]) + '.' + str(calibration_range[1]) + '.' +
  str(calibration_range[2]) + '.' + str(calibration_range[3])
"""


def setDstStoragePattern(pattern):
    global dst_storage_pattern
    dst_storage_pattern = pattern
"""
def createCalibration(name, importCalibration=None):
  if os.path.exists(name):
    print 'Calibration with global tag ', name, ' already exists. Re-loading'
  elif importCalibration is not None:
    import shutil
    shutil.copyfile(Belle2.FileSystem.findFile(importCalibration), name)
  set_global_tag(name)
"""


def selectRange(expMin, runMin, expMax, runMax):
    global calibration_range
    calibration_range = [expMin, runMin, expMax, runMax]


def selectSample(runs=[]):
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-nocal', '--no-calibrate', dest='nocalibrate', action='store_true', help='Do not perform calibration')
    parser.add_argument('-nocol', '--no-collect', dest='nocollect', action='store_true', help='Do not perform data collection')
    args = parser.parse_args()
    docollect = True
    docalibrate = True
    if (args.nocalibrate):
        docalibrate = False
    if (args.nocollect):
        docollect = False

    global calibration_range
    global dst_storage_pattern
    if dst_storage_pattern is None:
        raise EnvironmentError('No storage (and exp/run pattern) for DST files is defined.')
    filelist = []
    if calibration_range is None:
        raise EnvironmentError('Calibration range not selected. Use selectRange(...) before selectSample(...).')
    for exp in range(calibration_range[0], calibration_range[2] + 1):
        # Sorry to all who see this... I should scan the DST files first for exp,run
        # but as this is anyway temporary solution, I don't bother now
        # So use run numbers less than < 100 please
        runmin = 0
        runmax = 100
        if exp == calibration_range[0]:
            runmin = calibration_range[1]
        if exp == calibration_range[2]:
            runmax = calibration_range[3]
        for run in range(runmin, runmax + 1):
            if not len(runs) or runs.count((exp, run)):
                filename = dst_storage_pattern.format(experiment=exp, run=run)
                filename = Belle2.FileSystem.findFile(filename)
                if filename:
                    filelist.append(filename)
    input_path.add_module('RootInput', inputFileNames=filelist)
    input_path.add_module(
        'ConfigureCalibrationManager',
        rangeExpMin=calibration_range[0],
        rangeRunMin=calibration_range[1],
        rangeExpMax=calibration_range[2],
        rangeRunMax=calibration_range[3],
        doCalibration=docalibrate,
        doCollection=docollect)


def loadGeometry(geo_components=['MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD']):
    input_path.add_module('Gearbox')
    input_path.add_module('Geometry', components=geo_components)
    if len(magnet_off_run_list) != len(magnet_off_exp_list):
        raise AttributeError('Mismatch in list of runs and experiments where magnet is off')
    if len(magnet_off_exp_list):
        input_path.add_module('MagnetSwitcher', runList=magnet_off_run_list, expList=magnet_off_exp_list)


def useMisalignment(name):
    global misalignment_scenario_name
    misalignment_scenario_name = name


def setMagnetOffRuns(expruns):
    global magnet_off_exp_list
    global magnet_off_run_list
    for exprun in expruns:
        magnet_off_exp_list.append(exprun[0])
        magnet_off_run_list.append(exprun[1])


def refitGBL(UseClusters=False):
    gbl = register_module('GBLfit')
    gbl.param('UseClusters', UseClusters)
    if misalignment_scenario_name is not None:
        gbl.param('misalignment', misalignment_scenario_name)
    reprocessing_path.add_module(gbl)


def createPedeSteering(name='PedeSteering', commands=['method inversion 3 0.1'], fix=[]):
    steerer = register_module('PedeSteeringCreator')
    steerer.set_name(name)
    steerer.param('name', name)
    steerer.param('commands', commands)
    parameters = []
    for mask in fix:
        parameters.append(mask + ':0,-1')
    steerer.param('parameters', parameters)
    input_path.add_module(steerer)


def addMillepedeCalibration(name='MillepedeCalibration', granularity='data', dependency='', steering=''):
    def set_common_params(module):
        module.param('baseName', name)
        module.param('steering', steering)
        module.param('minPvalue', -1)
        module.param('granularityOfCalibration', granularity)

    collector = register_module('MillepedeCalibration')
    collector.set_name(name + '_collector')
    collector.param('isCalibrator', False)
    collector.param('Dependencies', name + ':running')
    set_common_params(collector)

    calibrator = register_module('MillepedeCalibration')
    calibrator.set_name(name)
    calibrator.param('isCollector', False)
    calibrator.param('Dependencies', dependency)
    set_common_params(calibrator)

    collection_path.add_module(collector)
    calibration_path.add_module(calibrator)


def commit():
    return


def calibrate():
    calibration_main.add_path(input_path)
    if not getCommandLineOptions().nocollect:
        calibration_main.add_path(reprocessing_path)
        calibration_main.add_path(collection_path)
    calibration_main.add_module('Progress')
    calibration_main.add_path(calibration_path)
    calibration_main.add_module('RootOutput', outputFileName='output.root', branchNames=['EventMetaData'])
    process(calibration_main)
    import os
    import sys
    # if not os.path.isfile('calibration_result.txt'):
    #  os.system('basf2 ' +  sys.argv[0] + '')
