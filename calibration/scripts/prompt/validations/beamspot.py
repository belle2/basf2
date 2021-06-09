# -*- coding: utf-8 -*-

'''
Validation of the Beam Spot calibration
'''


import basf2
from prompt import ValidationSettings
import ROOT
import sys
import subprocess
import json

import numpy as np
import scipy.linalg as la
import matplotlib.pyplot as plt

import re
import os
from glob import glob
from math import sqrt, frexp, asin

from datetime import datetime, timedelta
from ROOT.Belle2 import Unit

#: Tells the automated system some details of this script
settings = ValidationSettings(name='BeamSpot Calibrations',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={})


# Get the eigen parameters of the matrix
def getEigenPars(covM):
    sxx, syy, szz, sxy, sxz, syz = covM

    mat = np.array([[sxx, sxy, sxz],
                    [sxy, syy, syz],
                    [sxz, syz, szz]])

    result = la.eig(mat)
    eigVals = result[0].real
    eigVecs = result[1]

    # sort it by size
    idx = eigVals.argsort()[::-1]
    eigVals = eigVals[idx]
    eigVecs = eigVecs[:, idx]

    assert(eigVals[0] > 1e-4)
    assert(eigVals[1] > 1e-4)
    assert(eigVals[2] > 1e-4)

    sxEig = sqrt((eigVals[1]))
    syEig = sqrt((eigVals[2]))
    szEig = sqrt((eigVals[0]))

    # get the signs right
    if eigVecs[2][0] < 0:
        eigVecs[:, 0] *= -1

    if eigVecs[0][1] < 0:
        eigVecs[:, 1] *= -1

    if eigVecs[1][2] < 0:
        eigVecs[:, 2] *= -1

    # calculate the angles in mrad
    angleXZ = Unit.rad / Unit.mrad * asin(eigVecs[0][0] / sqrt(1 - eigVecs[1][0]**2))
    angleYZ = Unit.rad / Unit.mrad * asin(eigVecs[1][0])
    angleXY = Unit.rad / Unit.mrad * asin(eigVecs[1][1] / sqrt(1 - eigVecs[1][0]**2))

    return [sxEig, syEig, szEig, angleXZ, angleYZ, angleXY]


# Load the values of the Beam Spot parameters into the list
def getBSvalues(path):
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
        bsAll = f.Get("BeamSpot")
        assert(bsAll.ClassName() == "Belle2::EventDependency")

        evNums = bsAll.getEventNumbers()

        for i in range(len(evNums) + 1):
            bs = bsAll.getObjectByIndex(i)
            ipR = bs.getIPPosition()
            ip = [ipR(i) * Unit.cm / Unit.um for i in range(3)]  # from cm to um
            ipeR = bs.getIPPositionCovMatrix()
            ipe = [sqrt(ipeR(i, i)) * Unit.cm / Unit.um for i in range(3)]  # from cm to um
            covR = bs.getSizeCovMatrix()
            covM = (covR(0, 0), covR(1, 1), covR(2, 2), covR(0, 1), covR(0, 2), covR(1, 2))
            covM = [x * (Unit.cm / Unit.um)**2 for x in covM]  # from cm2 to um2

            tStart = ipeR(0, 1) * 1e20
            tEnd = ipeR(0, 2) * 1e20
            tStart = int(frexp(ipeR(0, 1))[0] * 2**53) % 2**32 / 3600.
            tEnd = int(frexp(ipeR(0, 2))[0] * 2**53) % 2**32 / 3600.

            eigM = getEigenPars(covM)
            arr.append((runExp, tStart, tEnd, ip, ipe, covM, eigM))
        f.Close()

    arr = sorted(arr, key=lambda x: x[1])

    return arr

# Print the Beam Spot parameters to the text file


def printToFile(arr):

    with open('bsData.csv', 'w') as outFile:
        outFile.write('exp  run  tStart  tEnd  xIP  yIP  zIP  xeIP  yeIP  zeIP  sXX  sYY  sZZ  sXY  sXZ  '
                      + 'sYZ  xSizeEig  ySizeEig  zSizeEig  xzAngle  yzAngle  xyAngle\n')

        for e in arr:
            outFile.write(
                # exp  run
                str(e[0][0]) + ' ' +
                str(e[0][1]) + ' ' +
                # tStart  tEnd
                str(e[1]) + ' ' +
                str(e[2]) + ' ' +
                # xIP  yIP  zIP
                str(e[3][0]) + ' ' +
                str(e[3][1]) + ' ' +
                str(e[3][2]) + ' ' +
                # xeIP  yeIP  zeIP
                str(e[4][0]) + ' ' +
                str(e[4][1]) + ' ' +
                str(e[4][2]) + ' ' +
                # sXX  sYY  sZZ  sXY  sXZ  sYZ
                str(e[5][0]) + ' ' +
                str(e[5][1]) + ' ' +
                str(e[5][2]) + ' ' +
                str(e[5][3]) + ' ' +
                str(e[5][4]) + ' ' +
                str(e[5][5]) + ' ' +
                # xSizeEig  ySizeEig  zSizeEig  xzAngle  yzAngle  xyAngle
                str(e[6][0]) + ' ' +
                str(e[6][1]) + ' ' +
                str(e[6][2]) + ' ' +
                str(e[6][3]) + ' ' +
                str(e[6][4]) + ' ' +
                str(e[6][5]) + '\n')
        return arr


# Create a plot with the specified variable
def plotVar(arr, limits, vName, getterV, getterE=None):

    tVals = []
    bsVals = []
    bsErrs = []

    tGapVals = []
    bsGapVals = []

    for i, el in enumerate(arr):
        s, e = el[1], el[2]
        s = datetime.utcfromtimestamp((s + 9) * 3600)  # Convert to the JST (+9 hours)
        e = datetime.utcfromtimestamp((e + 9) * 3600)
        tVals.append(s)
        tVals.append(e)
        bsVals.append(getterV(el))
        bsVals.append(getterV(el))

        if getterE is not None:
            bsErrs.append(getterE(el))
            bsErrs.append(getterE(el))

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
        bsVals.append(np.nan)
        if getterE is not None:
            bsErrs.append(np.nan)

        # store curve connecting gaps
        tGapVals.append(gS - timedelta(seconds=1))
        tGapVals.append(gS)
        tGapVals.append(gE)
        tGapVals.append(gE + timedelta(seconds=1))

        bsGapVals.append(np.nan)
        bsGapVals.append(getterV(arr[i]))
        bsGapVals.append(getterV(arr[i + 1]))
        bsGapVals.append(np.nan)

    plt.plot(tVals, bsVals, linewidth=2, color='C0')
    plt.plot(tGapVals, bsGapVals, linewidth=2, color='C0', alpha=0.35)

    bsVals = np.array(bsVals)
    bsErrs = np.array(bsErrs)

    if getterE is not None:
        plt.fill_between(tVals, bsVals - bsErrs, bsVals + bsErrs, alpha=0.2)

    plt.xlabel('time')
    if 'Angle' in vName:
        plt.ylabel(vName + ' [mrad]')
    else:
        plt.ylabel(vName + ' [um]')

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

    plt.hist(diffs, bins=np.linspace(-20, 20, 80), label=f'q1={round(q1,1)}, q2={round(q2,1)}')
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
    assert(len(dbFile) == 1)
    dbFile = dbFile[0]
    inputDir = dbFile[:dbFile.rfind('/')]

    arr = getBSvalues(inputDir)

    # print the results to the CSV file
    printToFile(arr)

    plt.figure(figsize=(18, 9))

    # plot the results
    for limits in allLimits:
        plotVar(arr, limits, 'xIP', lambda e: e[3][0], lambda e: e[4][0])
        plotVar(arr, limits, 'yIP', lambda e: e[3][1], lambda e: e[4][1])
        plotVar(arr, limits, 'zIP', lambda e: e[3][2], lambda e: e[4][2])

        plotVar(arr, limits, 'xSpotSize', lambda e: sqrt(e[5][0]))
        plotVar(arr, limits, 'ySpotSize', lambda e: sqrt(e[5][1]))
        plotVar(arr, limits, 'zSpotSize', lambda e: sqrt(e[5][2]))

        plotVar(arr, limits, 'xSpotSizeEig', lambda e: e[6][0])
        plotVar(arr, limits, 'ySpotSizeEig', lambda e: e[6][1])
        plotVar(arr, limits, 'zSpotSizeEig', lambda e: e[6][2])

        plotVar(arr, limits, 'xzSpotAngle', lambda e: e[6][3])
        plotVar(arr, limits, 'yzSpotAngle', lambda e: e[6][4])
        plotVar(arr, limits, 'xySpotAngle', lambda e: e[6][5])

    plotPullSpectrum(arr, 'xIPpulls', lambda e: e[3][0], lambda e: e[4][0])
    plotPullSpectrum(arr, 'yIPpulls', lambda e: e[3][1], lambda e: e[4][1])
    plotPullSpectrum(arr, 'zIPpulls', lambda e: e[3][2], lambda e: e[4][2])


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
