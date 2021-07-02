# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Validation of the Boost Vector calibration
'''


import basf2
from prompt import ValidationSettings
import ROOT
import sys
import subprocess
import json

import numpy as np
import scipy.linalg as la
import scipy.stats
import matplotlib.pyplot as plt

import re
import os
from glob import glob
from math import sqrt, frexp, asin, hypot, atan2

from datetime import datetime, timedelta
from ROOT.Belle2 import Unit

#: Tells the automated system some details of this script
settings = ValidationSettings(name='BoostVector Calibrations',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={})


# Get the eigen parameters of the matrix
def getBVvalsErrs(bv, bve):

    bv = np.array(bv)
    bve = np.array(bve)

    mag = np.linalg.norm(bv)
    xzAngle = atan2(bv[0], bv[2])
    yzAngle = atan2(bv[1], bv[2])

    magUnc = np.linalg.norm(bv * bve) / mag
    xzAngleUnc = 1. / (1 + (bv[0] / bv[2])**2) * hypot(bve[0] / bv[2], -bve[1] * bv[0] / bv[2]**2)
    yzAngleUnc = 1. / (1 + (bv[1] / bv[2])**2) * hypot(bve[1] / bv[2], -bve[1] * bv[1] / bv[2]**2)

    tomrad = Unit.rad / Unit.mrad
    return [1e3 * mag, tomrad * xzAngle, tomrad * yzAngle], [1e3 * magUnc, tomrad * xzAngleUnc, tomrad * yzAngleUnc]


# Load the values of the Boost Vector parameters into the list
def getBVvalues(path):
    runDict = {}
    with open(path + '/database.txt') as fDB:
        for ll in fDB:
            ll = ll.strip()
            ll = ll.split(' ')

            # deriving file name
            fN = ll[0].replace('/', '_') + '_rev_' + ll[1] + '.root'

            r = ll[2].split(',')
            runDict[fN] = (int(r[0]), int(r[1]))

    arr = []

    fList = glob(path + '/*.root')
    for fName in fList:
        bName = os.path.basename(fName)
        runExp = runDict[bName]

        f = ROOT.TFile.Open(fName)
        bvAll = f.Get("CollisionBoostVector")
        assert(bvAll.ClassName() == "Belle2::EventDependency")

        evNums = bvAll.getEventNumbers()

        for i in range(len(evNums) + 1):
            bvObj = bvAll.getObjectByIndex(i)

            bvV = bvObj.getBoost()
            bv = [bvV(i) for i in range(3)]
            bveV = bvObj.getBoostCovariance()
            bve = [sqrt(bveV(i, i)) for i in range(3)]

            tStart = int(frexp(bveV(0, 1))[0] * 2**53) % 2**32 / 3600.
            tEnd = int(frexp(bveV(0, 2))[0] * 2**53) % 2**32 / 3600.

            bvV, bvE = getBVvalsErrs(bv, bve)

            arr.append((runExp, tStart, tEnd, bvV, bvE))
        f.Close()

    arr = sorted(arr, key=lambda x: x[1])

    return arr

# Print the Boost Vector parameters to the text file


def printToFile(arr):

    with open('bvData.csv', 'w') as outFile:
        outFile.write('exp  run  tStart  tEnd  mag  xzAngle  yzAngle    magUnc  xzAngleUnc  yzAngleUnc \n')

        for e in arr:
            outFile.write(
                # exp  run
                str(e[0][0]) + ' ' +
                str(e[0][1]) + ' ' +
                # tStart  tEnd
                str(e[1]) + ' ' +
                str(e[2]) + ' ' +
                # mag  xzAngle  yzAngle
                str(e[3][0]) + ' ' +
                str(e[3][1]) + ' ' +
                str(e[3][2]) + ' ' +
                # magUnc  xzAngleUnc  yzAngleUnc
                str(e[4][0]) + ' ' +
                str(e[4][1]) + ' ' +
                str(e[4][2]) + '\n')
        return arr


# Create a plot with the specified variable
def plotVar(arr, limits, vName, getterV, getterE=None):

    tVals = []
    Vals = []
    Errs = []

    tGapVals = []
    GapVals = []

    for i, el in enumerate(arr):
        s, e = el[1], el[2]
        s = datetime.utcfromtimestamp((s + 9) * 3600)  # Convert to the JST (+9 hours)
        e = datetime.utcfromtimestamp((e + 9) * 3600)
        tVals.append(s)
        tVals.append(e)
        Vals.append(getterV(el))
        Vals.append(getterV(el))

        if getterE is not None:
            Errs.append(getterE(el))
            Errs.append(getterE(el))

        # Add breaks for longer gap if not the last interval
        if i >= len(arr) - 1:
            continue

        dt = (arr[i + 1][1] - arr[i][2]) * 3600

        # only consider gaps longer than 10 mins
        if dt < 10 * 60:
            continue

        # start-time of gap, end-time of gap
        gS = datetime.utcfromtimestamp((arr[i][2] + 9) * 3600)  # Convert to the JST (+9 hours)
        gE = datetime.utcfromtimestamp((arr[i + 1][1] + 9) * 3600)

        tVals.append(gS + (gE - gS) / 2)
        Vals.append(np.nan)
        if getterE is not None:
            Errs.append(np.nan)

        # store curve connecting gaps
        tGapVals.append(gS - timedelta(seconds=1))
        tGapVals.append(gS)
        tGapVals.append(gE)
        tGapVals.append(gE + timedelta(seconds=1))

        GapVals.append(np.nan)
        GapVals.append(getterV(arr[i]))
        GapVals.append(getterV(arr[i + 1]))
        GapVals.append(np.nan)

    plt.plot(tVals, Vals, linewidth=2, color='C0')
    plt.plot(tGapVals, GapVals, linewidth=2, color='C0', alpha=0.35)

    Vals = np.array(Vals)
    Errs = np.array(Errs)

    if getterE is not None:
        plt.fill_between(tVals, Vals - Errs, Vals + Errs, alpha=0.2)

    plt.xlabel('time')
    if 'Angle' in vName:
        plt.ylabel(vName + ' [mrad]')
    else:
        plt.ylabel(vName + r' $[10^{-3}]$')

    loc = 'plots/allData'
    if limits is not None:
        plt.xlim(datetime.strptime(limits[0], '%Y-%m-%d'), datetime.strptime(limits[1], '%Y-%m-%d'))
        loc = 'plots/' + limits[0] + 'to' + limits[1]

    # if not os.path.isdir(loc):
    os.makedirs(loc, exist_ok=True)

    plt.savefig(loc + '/' + vName + '.png')
    plt.clf()


def plotPullSpectrum(arr, vName, getterV, getterE):

    from itertools import groupby
    vals = np.array([k for k, g in groupby([getterV(v) for v in arr])])
    errs = np.array([k for k, g in groupby([getterE(v) for v in arr])])
    assert(len(vals) == len(errs))

    diffs = (vals[1:] - vals[:-1]) / np.hypot(errs[1:], errs[:-1])

    # Get 1sigma quantiles of the normal distribution
    n1, n2 = scipy.stats.norm.cdf([-1, 1])

    q1 = np.quantile(diffs, n1)
    q2 = np.quantile(diffs, n2)

    plt.hist(diffs, bins=np.linspace(-10, 10, 20), label=f'q1={round(q1,1)}, q2={round(q2,1)}')
    plt.xlabel(vName)
    plt.legend()

    loc = 'plots/allData'
    plt.savefig(loc + '/' + vName + '.png')
    plt.clf()


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    '''
    Run the validation.
    '''

    # Expert config can contain the time ranges of the plots
    if expert_config != '':
        expert_config = json.loads(expert_config)

    allLimits = [None]
    if expert_config is not None and 'plotsRanges' in expert_config:
        allLimits += expert_config['plotsRanges']

    # Path to the database.txt file and to the payloads.
    dbFile = glob(f'{job_path}/**/database.txt', recursive=True)
    dbFile = [db for db in dbFile if 'algorithm_output' not in db]
    assert(len(dbFile) == 1)
    dbFile = dbFile[0]
    inputDir = dbFile[:dbFile.rfind('/')]

    arr = getBVvalues(inputDir)

    # print the results to the CSV file
    printToFile(arr)

    plt.figure(figsize=(18, 9))

    # plot the results
    for limits in allLimits:
        plotVar(arr, limits, 'boostMag', lambda e: e[3][0], lambda e: e[4][0])
        plotVar(arr, limits, 'xzBoostAngle', lambda e: e[3][1], lambda e: e[4][1])
        plotVar(arr, limits, 'yzBoostAngle', lambda e: e[3][2], lambda e: e[4][2])

    plotPullSpectrum(arr, 'boostMagPulls', lambda e: e[3][0], lambda e: e[4][0])
    plotPullSpectrum(arr, 'xzBoostAnglePulls', lambda e: e[3][1], lambda e: e[4][1])
    plotPullSpectrum(arr, 'yzBoostAnglePulls', lambda e: e[3][2], lambda e: e[4][2])


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
