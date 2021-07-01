#!/usr/bin/env python

# Import bunch structure payload for master GT.
# Create payload taking information from the RunDB.
# Usage: basf2 DBBunchStructureImporter.py <minexp> <minrun> <maxexp> <maxrun>
#
# if minexp is 0 the designed filling patter will be created. Meaning 10101010...
# if minexp is 1003 the early phase 3 pattern will be created. Take the most used filling pattern.

from ROOT import Belle2
from rundb import RunDB
import sys
import basf2


def fill(fillPatternHex, firstExp, firstRun, lastExp, lastRun):
    """Create a payload with the given hexadecmal fill pattern for the given run range"""

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

    extraStep = 0
    for b in range(0, 5120, 2):

        # Shift of 1 bunch every 300 bunches to simulate bunch trains
        if((b % 300 == 0) & (b != 0)):
            extraStep += 1

        bunches.setBucket(b + extraStep)

    iov = Belle2.IntervalOfValidity(0, 0, 0, -1)

    db = Belle2.Database.Instance()
    db.storeData("BunchStructure", bunches, iov)

elif(minexp == "1003"):

    username = input("Enter your username: ")
    rundb = RunDB(username=username)

    runInfo = rundb.get_run_info(
        min_experiment=12,
        max_experiment=12,
        min_run=1859,
        max_run=1859,
        run_type='physics',
        expand=True
    )

    for it in runInfo:
        pattern = it['ler']['fill_pattern']
    fill(pattern, 1003, 0, 1003, -1)


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

    lumiMax = 0
    runExpMaxBegin = 0
    runExpMaxEnd = 0

    lumi = 0
    for it in runInfo:
        exprun = it['experiment'], it['run']
        pattern = it['ler']['fill_pattern']

        if(pattern == ""):
            continue

        # pattern different to previous one or first run
        if pattern != current_pattern:

            if(lumi > lumiMax):
                runExpMaxBegin = current_start
                runExpMaxEnd = current_end
                lumiMax = lumi

            # close the iov
            fill(current_pattern, *current_start, *current_end)
            if current_pattern is not None:
                print(f"Corresponding to {lumi/1000.:.2f} pb-1\n")

            # and remember new values
            current_pattern = pattern
            current_start = exprun
            current_end = exprun

            lumi = it['statistics']['lumi_recorded']

        else:
            # pattern unchanged, extend current iov
            current_end = exprun
            lumi += it['statistics']['lumi_recorded']

    # close the last iov if any
    fill(current_pattern, *current_start, *current_end)
    print(f"Corresponding to {lumi/1000.:.2f} pb-1\n")

    print(
        f"Most used filling pattern:\niov: \
        {runExpMaxBegin[0]},{runExpMaxBegin[1]},{runExpMaxEnd[0]},{runExpMaxEnd[1]} \
        \nIntegrated luminosity: {lumiMax/1000:.2f} pb-1")
