#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#####################################################################
# ECL local run calibration:                                        #
# run_eclLocalRunCalibGenRunTable.py                                #
#                                                                   #
# It is a script, which can be used to create run time table.       #
#                                                                   #
# Author: The Belle II Collaboration                                #
# Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP),   #
#                                                                   #
#####################################################################
import re
import os
import sys
import time
import subprocess
import json


def getRunSubDirs(path):
    lst = next(os.walk(path))[1]
    slst = sorted([x for x in lst if x.isdigit()])
    return slst


def getLocalRunFileParams(path):
    lst = next(os.walk(path))[2]
    template = 'ecl.([0-9]{4}).([0-9]{5}).HLT1.f00000.sroot'
    p = re.compile(template)
    flst = [x for x in lst if p.match(x)]
    if len(flst) != 1:
        return False
    else:
        res = re.search(p, flst[0])
        flpath = os.path.join(path, flst[0])
        epochTime = os.path.getctime(flpath)
        return {'exp': res.group(1),
                'run': res.group(2),
                'time': epochTime}


def getRunTable(path):
    dirlist = getRunSubDirs(path)
    res = []
    for subdir in dirlist:
        spath = os.path.join(path, subdir)
        par = getLocalRunFileParams(spath)
        if par:
            res.append(par)
    return res


def main():
    path = sys.argv[1]
    lst = getRunTable(path)
    lst = sorted(lst, key=lambda x: x['time'])
    print('exp/I:run/I:run_start/C:run_end/C:evt_count/I')
    for el in lst:
        sqlTimeEnd = sqlTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(el['time']))
        sqlTimeBegin = sqlTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(el['time'] - 1))
        print('%d, %d, %s, %s, %d' % (int(el['exp']), int(el['run']), sqlTimeBegin, sqlTimeEnd, 1000))


if __name__ == '__main__':
    main()
