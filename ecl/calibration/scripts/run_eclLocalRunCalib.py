#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#####################################################################
#                                                                   #
# run_eclLocalRunCalib.py                                           #
#                                                                   #
# It is a simple script used to run ecl local run calibration.      #
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
    dirname = os.path.dirname(sys.argv[0])
    filename = sys.argv[1]
    cmd = 'python3 '
    cmd += os.path.join(dirname, 'eclLocalRunCalib.py')
    cmd += ' %s' % (loadOptions(),)
    cmd += ' --filename %s' % (filename,)
    if len(sys.argv) != 3 or sys.argv[2] != 'start':
        cmd += ' --addref --changeprev'
    return cmd


def main():
    cmd = getCommand()
    subprocess.call(cmd, shell=True)


if __name__ == '__main__':
    main()
