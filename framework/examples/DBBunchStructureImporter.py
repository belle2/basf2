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


def fill(fillPatternHex, firstExp, firstRun, lastExp, lastRun):

    fillPatternBin = bin(int(theFillPatternHex, 16))
    fillPatternBin = fillPatternBin.replace('0b', '')

    for num, bucket in enumerate(fillPatternBin):
        if(bucket == "1"):
            bunches.setBucket(num)

    print("Filling new payload. iov:", firstExp, firstRun, currentExp, currentRun)

    iov = Belle2.IntervalOfValidity(firstExp, firstRun, currentExp, currentRun)

    db = Belle2.Database.Instance()
    db.storeData("BunchStructure", bunches, iov)


# Argument parsing
argvs = sys.argv


if len(argvs) == 2:
    minexp = argvs[1]

elif len(sys.argv) == 5:
    minexp = argvs[1]
    minrun = argvs[2]
    maxexp = argvs[3]
    maxrun = argvs[4]
else:
    basf2.B2ERROR("Wrong arguments. Options are:\n",
                  "1 arguments usage: basf2 ", argvs[0], " <0> or <1003>\n"
                  "4 arguments usage: basf2 ", argvs[0], " <minexp> <minrun> <maxexp> <maxrun>")
    sys.exit()


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
    username = input("Enter your username: ")
    rundb = RunDB(username=username)

    runInfo = rundb.get_run_info(
        min_experiment=minexp,
        max_experiment=maxexp,
        min_run=minrun,
        max_run=maxrun,
        expand=True
    )

    theFillPatternHex = []
    newiov = True

    currentRun = minexp
    currentExp = minrun

    for it in runInfo:

        bunches = Belle2.BunchStructure()
        fillPatternHex = it['ler']['fill_pattern']

        if(it['run_type'] != 'physics'):
            continue

        currentExp = it['experiment']
        currentRun = it['run']

        if(fillPatternHex == theFillPatternHex or newiov):
            theFillPatternHex = fillPatternHex

            if(newiov):
                firstExp = int(it['experiment'])
                firstRun = int(it['run'])
                newiov = False

        if(fillPatternHex != theFillPatternHex):
            fill(theFillPatternHex, firstExp, firstRun, currentExp, currentRun)

            newiov = True

    # in order to close the db with last run
    if(fillPatternHex == theFillPatternHex):
        fill(theFillPatternHex, firstExp, firstRun, currentExp, currentRun)
