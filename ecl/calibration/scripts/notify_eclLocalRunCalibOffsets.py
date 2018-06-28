#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#####################################################################
# ECL local run calibration:                                        #
# notify_eclLocalRunCalibOffsets.py                                 #
#                                                                   #
# It is as simple script, which allows user to check                #
# notifications about large offsets.                                #
#                                                                   #
# Default database: localdb/database.txt                            #
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
        return opts


def getCommand():
    cmd = 'eclLocalRunCalibNotify %s \
     --exp %s --run %s --offset' % (
        loadOptions(), sys.argv[1], sys.argv[2])
    return cmd


def main():
    cmd = getCommand()
    subprocess.call(cmd, shell=True)


if __name__ == '__main__':
    main()
