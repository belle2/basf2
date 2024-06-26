#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
import collections


def getCollidingPatternFromFiles(herFilePath, lerFilePath):
    """
    Take HER and LER filling pattern from file and perform AND operation to take only colliding buckets.

    The format is the one given by the accelerator people. The first column is the number of the bunch,
    the second is 1.0000 or .0000 depending on whether the bunch is filled or not. The third column is extra.
    """

    herFile = open(herFilePath)
    herLines = herFile.readlines()

    lerFile = open(lerFilePath)
    lerLines = lerFile.readlines()

    pattern = []

    for lineHer, lineLer in zip(herLines, lerLines):
        if(lineHer[0] == "#"):
            continue

        if(lineHer.split()[1] == '1.0000' and lineLer.split()[1] == '1.0000'):
            pattern.append(1)
        else:
            pattern.append(0)

    return(pattern)


def getCollidingPattern(herFPHex, lerFPHex):
    """Take LER and HER hexadecimal fill pattern, convert to binary and perform AND operation to take only colliding buckets"""

    herFPBin = bin(int(herFPHex, 16))
    herFPBin = herFPBin.replace('0b', '')

    lerFPBin = bin(int(lerFPHex, 16))
    lerFPBin = lerFPBin.replace('0b', '')

    return [int(h) & int(l) for h, l in zip(herFPBin, lerFPBin)]


def fill(fillPattern, firstExp, firstRun, lastExp, lastRun):
    """Create a payload fill pattern for the given run range"""

    if fillPattern is None:
        return

    bunches = Belle2.BunchStructure()

    if fillPattern != "":
        for num, bucket in enumerate(fillPattern):
            if(bucket):
                bunches.setBucket(num)

    iov = Belle2.IntervalOfValidity(firstExp, firstRun, lastExp, lastRun)

    db = Belle2.Database.Instance()

    db.storeData("BunchStructure", bunches, iov)

    if fillPattern != "":
        basf2.B2INFO(f"Filling new payload. iov: {firstExp} {firstRun} {lastExp} {lastRun}")
    else:
        basf2.B2INFO(f"Filling new payload with default filling pattern. iov:{firstExp} {firstRun} {lastExp} {lastRun}\n")


# Argument parsing
argvs = sys.argv


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Create bunch structure payload taking information from the RunDB')
    parser.add_argument('minExp', metavar='minExp', type=int,
                        help='first experiment. Use only this argument to produce filling pattern for experiment 0 or 1003')
    parser.add_argument('minRun', metavar='minRun', type=int, nargs='?',
                        help='first run')
    parser.add_argument('maxExp', metavar='maxExp', type=int, nargs='?',
                        help='last experiment')
    parser.add_argument('maxRun', metavar='maxRun', type=int, nargs='?',
                        help='last run')
    parser.add_argument(
        '--exprun1003',
        dest='exprun1003',
        metavar='<exp> <run>',
        nargs='+',
        default=[
            12,
            1859],
        type=int,
        help='Change run and experiment of the filling pattern to be used for early phase 3 \
        run-independent MC (exp 1003). (default: %(default)s)')

    parser.add_argument(
        '--patternFromFiles',
        dest='patternFromFiles',
        metavar='<herFile> <lerFile>',
        nargs='+',
        default=None,
        help='Use external files to create filling pattern. The format is the one given by accelerator group')

    parser.add_argument(
        '--dontFillGaps',
        dest='dontFillGaps',
        action='store_true',
        default=False,
        help="Don't fill the gaps between physic iovs with default filling pattern")

    args = parser.parse_args()

    if(args.minExp == 0):
        bunches = Belle2.BunchStructure()

        if(args.patternFromFiles):
            basf2.B2INFO("Using external files")
            pattern = getCollidingPatternFromFiles(args.patternFromFiles[0], args.patternFromFiles[1])
            fill(pattern, 0, 0, 0, -1)

        else:
            extraStep = 0
            for b in range(0, 5120, 2):

                # Shift of 1 bunch every 300 bunches to simulate bunch trains
                if((b % 300 == 0) & (b != 0)):
                    extraStep += 1

                bunches.setBucket(b + extraStep)

            iov = Belle2.IntervalOfValidity(0, 0, 0, -1)

            db = Belle2.Database.Instance()
            db.storeData("BunchStructure", bunches, iov)

    elif(args.minExp == 1003):

        if(args.patternFromFiles):
            basf2.B2INFO("Using external files")
            pattern = getCollidingPatternFromFiles(args.patternFromFiles[0], args.patternFromFiles[1])

        else:
            username = input("Enter your DESY username:")
            rundb = RunDB(username=username)

            basf2.B2INFO(f"Use exp {args.exprun1003[0]} and run {args.exprun1003[1]} for early phase 3 run-independent MC")

            runInfo = rundb.get_run_info(
                min_experiment=args.exprun1003[0],
                max_experiment=args.exprun1003[0],
                min_run=args.exprun1003[1],
                max_run=args.exprun1003[1],
                run_type='physics',
                expand=True
            )

            for it in runInfo:
                pattern = getCollidingPattern(it['her']['fill_pattern'], it['ler']['fill_pattern'])

        fill(pattern, 1003, 0, 1003, -1)

    elif(args.patternFromFiles):
        basf2.B2INFO("Using external files")
        pattern = getCollidingPatternFromFiles(args.patternFromFiles[0], args.patternFromFiles[1])
        fill(pattern, args.minExp, args.minRun, args.maxExp, args.maxRun)

    else:
        username = input("Enter your DESY username: ")
        rundb = RunDB(username=username)

        minexp = args.minExp
        minrun = args.minRun
        maxexp = args.maxExp
        maxrun = args.maxRun

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

        # Necessary to fill first gap
        current_end = minexp, minrun

        # Dictionary used to calculate integrated lumi for each filling pattern and store info for summary
        lumiDic = {}

        lastEnd = minexp, minrun

        lumi = 0

        for it in runInfo:
            exprun = it['experiment'], it['run']

            if(it['her']['fill_pattern'] == "" or it['ler']['fill_pattern'] == ""):
                basf2.B2WARNING(f"Filling pattern for Exp {exprun[0]} Run {exprun[1]} is empty. \
                Previous filling pattern will be used instead.")
                continue

            pattern = getCollidingPattern(it['her']['fill_pattern'], it['ler']['fill_pattern'])

            # pattern different to previous one or first run
            if pattern != current_pattern:

                if(args.dontFillGaps is False and current_start[0] is not None
                   and current_start[1] != lastEnd[1] + 1 and current_start[1] != lastEnd[1]):
                    fill("", lastEnd[0], lastEnd[1] + 1, current_start[0], current_start[1] - 1)

                # close the iov.  If it's first run fill() doesn't close the iov.
                fill(current_pattern, *current_start, *current_end)

                if current_pattern is not None:
                    basf2.B2INFO(f"Corresponding to {lumi/1000.:.2f} pb-1\n")
                    # fill iovs in lumi dic
                    lumiDic[''.join(str(i) for i in current_pattern)]["iovs"].append([*current_start, *current_end])

                # Create dictionary keys using the filling pattern itself
                keyFP = ''.join(str(i) for i in pattern)

                if(keyFP in lumiDic):
                    lumiDic[keyFP]["lumi"] += it['statistics']['lumi_recorded']

                else:
                    lumiDic[keyFP] = {}
                    lumiDic[keyFP]["lumi"] = it['statistics']['lumi_recorded']
                    lumiDic[keyFP]["lerBunches"] = it['ler']['number_of_bunches']
                    lumiDic[keyFP]["herBunches"] = it['her']['number_of_bunches']
                    lumiDic[keyFP]["iovs"] = []

                lumi = it['statistics']['lumi_recorded']

                # last values
                lastEnd = current_end

                # and remember new values
                current_pattern = pattern
                current_start = exprun
                current_end = exprun

            else:
                # pattern unchanged, extend current iov
                current_end = exprun
                lumi += it['statistics']['lumi_recorded']
                lumiDic[''.join(str(i) for i in current_pattern)]["lumi"] += it['statistics']['lumi_recorded']

        # close the last iov if any
        if(args.dontFillGaps is False and current_start[0] is not None and current_start[1] != lastEnd[1] + 1):
            fill("", lastEnd[0], lastEnd[1] + 1, current_start[0], current_start[1] - 1)

        fill(current_pattern, *current_start, *current_end)
        basf2.B2INFO(f"Corresponding to {lumi/1000.:.2f} pb-1\n")
        lumiDic[''.join(str(i) for i in current_pattern)]["iovs"].append([*current_start, *current_end])

        if(args.dontFillGaps is False and current_end[1] != maxrun):
            fill("", current_end[0], current_end[1] + 1, maxexp, maxrun)

        # last open iov with default filling pattern
        fill("", maxexp, maxrun + 1, -1, -1)

        # print a summary with info on most used filling pattern
        basf2.B2INFO("Summary:\n")
        i = 0
        for k, v in collections.OrderedDict(sorted(lumiDic.items(), key=lambda t: t[1]["lumi"], reverse=True)).items():
            i += 1
            basf2.B2INFO(f"{i} Filling pattern")
            basf2.B2INFO(f"  lumi {v['lumi']/1000.:.2f} pb-1")
            basf2.B2INFO(f"  HER/LER number of bunches {int(lumiDic[keyFP]['herBunches'])}/{int(lumiDic[keyFP]['lerBunches'])}")
            for iov in v["iovs"]:
                basf2.B2INFO(f"  iov {iov[0]} {iov[1]} - {iov[2]} {iov[3]}")
            print("")
