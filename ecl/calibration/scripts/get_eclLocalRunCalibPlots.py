#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#####################################################################
# ECL local run calibration:                                        #
# get_eclLocalRunCalibPlots.py                                      #
#                                                                   #
# It is as simple script, which allows user to get plots            #
# for a single calibration run.                                     #
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
    cmd = 'eclLocalRunCalibAnalyzer %s --exp %s --run %s' % (
        loadOptions(), sys.argv[1], sys.argv[2])
    return cmd


def main():
    cmd = getCommand()
    subprocess.call(cmd, shell=True)


if __name__ == '__main__':
    main()
