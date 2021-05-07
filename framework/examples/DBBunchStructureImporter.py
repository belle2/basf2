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
import sys
import basf2


def fill(fillPatternHex, firstExp, firstRun, lastExp, lastRun):

    if fillPatternHex is None:
        return

    bunches = Belle2.BunchStructure()

    fillPatternBin = bin(int(fillPatternHex, 16))
    fillPatternBin = fillPatternBin.replace('0b', '')

    for num, bucket in enumerate(fillPatternBin):
        if(bucket == "1"):
            bunches.setBucket(num)

    print("Filling new payload. iov:", firstExp, firstRun, lastExp, lastRun)

    iov = Belle2.IntervalOfValidity(firstExp, firstRun, lastExp, lastRun)

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

    for b in range(0, 5120, 2):
        bunches.setBucket(b)

    iov = Belle2.IntervalOfValidity(0, 0, 0, -1)

    db = Belle2.Database.Instance()
    db.storeData("BunchStructure", bunches, iov)

elif(minexp == "1003"):
    bunches = Belle2.BunchStructure()

    for b in range(0, 5120, 3):
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
        run_type='physics',
        expand=True
    )

current_pattern = None
current_start = None, None
current_end = None, None

for it in runInfo:

    exprun = it['experiment'], it['run']
    pattern = it['ler']['fill_pattern']

    # pattern different to previous one or first run
    if pattern != current_pattern:
        # close the last iov if any
        fill(current_pattern, *current_start, *current_end)
        # and remember new values
        current_pattern = pattern
        current_start = exprun
        current_end = exprun
    else:
        # pattern unchanged, extend current iov
        current_end = exprun

# close the last iov if any
fill(current_pattern, *current_start, *current_end)
