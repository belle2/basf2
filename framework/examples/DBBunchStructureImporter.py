#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Import bunch structure payload for master GT.
Create payload taking information from the RunDB.
Usage: basf2 DBBunchStructureImporter.py <username> <minexp> <minrun> <maxexp> <maxrun>

if minexp is 0 the designed filling patter will be created. Meaning 10101010...
if minexp is 1003 the early phase 3 pattern will be created. Meaning 1001001001..

"""

from ROOT import Belle2
from rundb import RunDB
import pandas as pd
from optparse import OptionParser
import sys
import basf2

# Argument parsing
argvs = sys.argv

if(len(argvs) < 6):
    basf2.B2ERROR("Wrong arguments. Usage: basf2 ", argvs[0], " <username> <minexp> <minrun> <maxexp> <maxrun>")
    sys.exit()

minexp = argvs[2]
minrun = argvs[3]
maxexp = argvs[4]
maxrun = argvs[5]

if(minexp == "0"):

    bunches = Belle2.BunchStructure()

    for b in range(0, 5120):
        if(b % 2 == 0):
            bunches.setBucket(b)

    iov = Belle2.IntervalOfValidity(0, 0, 0, -1)

    db = Belle2.Database.Instance()
    db.storeData("BunchStructure", bunches, iov)

elif(minexp == "1003"):
    bunches = Belle2.BunchStructure()

    for b in range(0, 5120):
        if(b % 3 == 0):
            bunches.setBucket(b)

    iov = Belle2.IntervalOfValidity(1003, 0, 1003, -1)

    db = Belle2.Database.Instance()
    db.storeData("BunchStructure", bunches, iov)

else:

    rundb = RunDB(username=sys.argv[1])

    res = rundb.get_run_info(
        min_experiment=minexp,
        max_experiment=maxexp,
        min_run=minrun,
        max_run=maxrun,
        expand=True
    )

    theFillPatternHex = []

    newiov = True

    for it in res:

        bunches = Belle2.BunchStructure()

        fillPatternHex = it['ler']['fill_pattern']

        if(fillPatternHex == theFillPatternHex or newiov):
            theFillPatternHex = fillPatternHex

            if(newiov):
                firstExp = int(it['experiment'])
                firstRun = int(it['run'])
                newiov = False

        if((it['run'] == int(maxrun) and it['experiment'] == int(maxexp)) or fillPatternHex != theFillPatternHex):
            lastExp = int(it['experiment'])
            lastRun = int(it['run'])
            newiov = True

            fillPatternBin = bin(int(theFillPatternHex, 16))

            fillPatternBin = fillPatternBin.replace('0b', '')

            for num, bucket in enumerate(fillPatternBin):
                if(bucket == "1"):
                    bunches.setBucket(num)

            print("iov:", firstExp, firstRun, lastExp, lastRun)

            iov = Belle2.IntervalOfValidity(firstExp, firstRun, lastExp, lastRun)

            db = Belle2.Database.Instance()
            db.storeData("BunchStructure", bunches, iov)
