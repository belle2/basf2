#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#####################################################################
# ECL local run calibration:                                        #
# get_eclLocalRunCalibTrendPlots.py                                 #
#                                                                   #
# It is as simple script, which allows user to get trend plots.     #
#                                                                   #
# Author: The Belle II Collaboration                                #
# Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP),   #
#                                                                   #
#####################################################################
import os
import sys
import subprocess
import json


def loadOptions():
    dirname = os.path.dirname(sys.argv[0])
    optfile = os.path.join(dirname, 'options_eclLocalRunCalib.json')
    with open(optfile) as fl:
        data = json.load(fl)
        opts = '--dbname %s' % (
            data['dboptions']['dbname'],)
        if data['dboptions']['centraldb']:
            opts += ' --centraldb'
        if data['reference']:
            opts += ' --withref'
        return opts


def getCommand():
    beginTime = '_'.join(sys.argv[1:3])
    endTime = '_'.join(sys.argv[3:5])
    cmd = 'eclLocalRunCalibAnalyzer %s \
     --timestart %s --timestop %s' % (
        loadOptions(), beginTime, endTime)
    if len(sys.argv) == 6:
        cmd += ' --cellid ' + sys.argv[5]
    return cmd


def main():
    if len(sys.argv) == 5 or len(sys.argv) == 6:
        cmd = getCommand()
        subprocess.call(cmd, shell=True)
    else:
        print('Atguments:\n'
              '<begin date> <begin time> <end date> <end time> (<cellid>)')


if __name__ == '__main__':
    main()
