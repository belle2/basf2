#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#####################################################################
# ECL local run calibration:                                        #
# run_eclLocalRunCalibProcessPath.py                                #
#                                                                   #
# It is a simple script used to run ecl local run calibration of    #
# all runs in a certain experiment.                                 #
# Default database: localdb/database.txt                            #
#                                                                   #
# Author: The Belle II Collaboration                                #
# Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP),   #
#                                                                   #
#####################################################################
import re
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


def getCommand(filename):
    dirname = os.path.dirname(sys.argv[0])
    executable = os.path.join(dirname, 'eclLocalRunCalib.py')
    cmd = executable + ' --filename ' + filename
    cmd += ' %s' % (loadOptions(),)
    return cmd


def getRunSubDirs(path):
    lst = next(os.walk(path))[1]
    slst = sorted([x for x in lst if x.isdigit()])
    return slst


def getLocalRunFile(path):
    lst = next(os.walk(path))[2]
    template = 'ecl.([0-9]{4}).([0-9]{5}).HLT1.f00000.sroot'
    p = re.compile(template)
    flst = [x for x in lst if p.match(x)]
    if len(flst) != 1:
        return False
    else:
        return flst[0]


def getLocalRunFileList(path):
    dirlist = getRunSubDirs(path)
    fllist = []
    for subdir in dirlist:
        spath = os.path.join(path, subdir)
        fl = getLocalRunFile(spath)
        if fl:
            fllist.append(os.path.join(spath, fl))
    return fllist


def processSortedFileList(lst):
    start = False
    for fl in lst:
        cmd = getCommand(fl)
        if not start:
            start = True
        else:
            cmd += ' --changeprev'
        print(cmd)
        subprocess.call(cmd, shell=True)


def main():
    path = sys.argv[1]
    lst = getLocalRunFileList(path)
    print('Processing %d files...' % (len(lst),))
    processSortedFileList(lst)


if __name__ == '__main__':
    main()
