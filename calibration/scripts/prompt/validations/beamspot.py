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

import matplotlib.pyplot as plt
import numpy as np

import re
import os
from glob import glob
from math import sqrt, frexp, asin


#: Tells the automated system some details of this script
settings = ValidationSettings(name='Beam Spot values',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={})


# Get the eigen parameters of the matrix
def getEigenPars(SizeM):
    sxx, syy, szz, sxy, sxz, syz = SizeM

    mat = np.array([[sxx, sxy, sxz],
                    [sxy, syy, syz],
                    [sxz, syz, szz]])

    import scipy.linalg as la
    result = la.eig(mat)
    eigVals = result[0].real
    eigVecs = result[1]

    # sort it by size
    idx = eigVals.argsort()[::-1]
    eigVals = eigVals[idx]
    eigVecs = eigVecs[:, idx]

    eigVals[1] = max(0.0001, eigVals[1])
    eigVals[2] = max(0.0001, eigVals[2])

    sxEig = sqrt((eigVals[1]))
    syEig = sqrt((eigVals[2]))
    szEig = sqrt((eigVals[0]))

    # get the signs right
    if eigVecs[2][0] < 0:
        eigVecs[0][0] *= -1
        eigVecs[1][0] *= -1
        eigVecs[2][0] *= -1

    if eigVecs[0][1] < 0:
        eigVecs[0][1] *= -1
        eigVecs[1][1] *= -1
        eigVecs[2][1] *= -1

    if eigVecs[1][2] < 0:
        eigVecs[0][2] *= -1
        eigVecs[1][2] *= -1
        eigVecs[2][2] *= -1

    # calculate the angles
    angleXZ = 1e3 * asin(eigVecs[0][0] / sqrt(1 - eigVecs[1][0]**2))
    angleYZ = 1e3 * asin(eigVecs[1][0])
    angleXY = 1e3 * asin(eigVecs[1][1] / sqrt(1 - eigVecs[1][0]**2))

    return [sxEig, syEig, szEig, angleXZ, angleYZ, angleXY]


# Load the values of the Beam Spot parameters into the list
def getBSvalues(path):
    runDict = {}
    fDB = open(path + '/database.txt')
    for ll in fDB:
        ll = ll.strip()
        ll = ll.split(' ')
        # print(ll)
        i = int(ll[1])
        r = ll[2].split(',')
        assert(r[0] == r[2])
        assert(r[1] == r[3])
        # print(r[0], r[1])
        runDict[int(i)] = (int(r[0]), int(r[1]))

    arr = []

    fList = glob(path + '/*.root')
    for fName in fList:
        r = re.findall("[0-9]+", os.path.basename(fName))
        Id = int(r[0])
        f = ROOT.TFile.Open(fName)

        bsAll = f.Get("BeamSpot")

        evNums = bsAll.getEventNumbers()

        for i in range(len(evNums) + 1):
            bs = bsAll.getObjectByIndex(i)
            ipV = bs.getIPPosition()
            ip = [1e4 * ipV(i) for i in range(3)]
            ipeV = bs.getIPPositionCovMatrix()
            ipe = [1e4 * sqrt(ipeV(i, i)) for i in range(3)]
            covM = bs.getSizeCovMatrix()
            sizeM = (covM(0, 0), covM(1, 1), covM(2, 2), covM(0, 1), covM(0, 2), covM(1, 2))
            sizeM = [1e8 * x for x in sizeM]

            tStart = ipeV(0, 1) * 1e20
            tEnd = ipeV(0, 2) * 1e20
            tStart = int(frexp(ipeV(0, 1))[0] * 2**53) % 2**32 / 3600.
            tEnd = int(frexp(ipeV(0, 2))[0] * 2**53) % 2**32 / 3600.

            eigM = getEigenPars(sizeM)
            arr.append((runDict[Id], tStart, tEnd, ip, ipe, sizeM, eigM))

    arr = sorted(arr, key=lambda x: x[1])

    return arr

# Print the Beam Spot parameters to the text file


def printToFile(arr):

    outFile = open('bsData.csv', 'w')
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
def plotVar(arr, vName, getterV, getterE=None):

    tVals = []
    bsVals = []
    bsErrs = []
    for i, el in enumerate(arr):
        s, e = el[1], el[2]
        from datetime import datetime, timedelta
        s = datetime.utcfromtimestamp((s + 9) * 3600)
        e = datetime.utcfromtimestamp((e + 9) * 3600)
        tVals.append(s)
        tVals.append(e)
        bsVals.append(getterV(el))
        bsVals.append(getterV(el))

        if getterE is not None:
            bsErrs.append(getterE(el))
            bsErrs.append(getterE(el))

        # Add breaks if gap more than 180s
        if i < len(arr) - 1:
            dt = (arr[i + 1][1] - arr[i][2]) * 3600
            if dt > 180:
                tVals.append(e + timedelta(seconds=90))
                bsVals.append(np.nan)
                if getterE is not None:
                    bsErrs.append(np.nan)

    plt.figure(figsize=(12, 6))
    plt.plot(tVals, bsVals)

    bsVals = np.array(bsVals)
    bsErrs = np.array(bsErrs)

    if getterE is not None:
        plt.fill_between(tVals, bsVals - bsErrs, bsVals + bsErrs, alpha=0.2)

    plt.xlabel('time')
    if 'angle' in vName:
        plt.ylabel(vName + ' [urad]')
    else:
        plt.ylabel(vName + ' [um]')

    plt.savefig(vName + '.png')


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    '''
    Run the validation.
    '''

    # Expert config is currently dummy
    expert_config = json.loads(expert_config)

    # Path to the database.txt file and to the payloads.
    inputDir = f'{job_path}/BeamSpot/outputdb'
    arr = getBSvalues(inputDir)

    # print the results to the CSV file
    printToFile(arr)

    # plot the results
    plotVar(arr, 'xIP', lambda e: e[3][0], lambda e: e[4][0])
    plotVar(arr, 'yIP', lambda e: e[3][1], lambda e: e[4][1])
    plotVar(arr, 'zIP', lambda e: e[3][2], lambda e: e[4][2])

    plotVar(arr, 'xSize', lambda e: sqrt(e[5][0]))
    plotVar(arr, 'ySize', lambda e: sqrt(e[5][1]))
    plotVar(arr, 'zSize', lambda e: sqrt(e[5][2]))

    plotVar(arr, 'xSizeEig', lambda e: e[6][0])
    plotVar(arr, 'ySizeEig', lambda e: e[6][1])
    plotVar(arr, 'zSizeEig', lambda e: e[6][2])

    plotVar(arr, 'xzAngle', lambda e: e[6][3])
    plotVar(arr, 'yzAngle', lambda e: e[6][4])
    plotVar(arr, 'xyAngle', lambda e: e[6][5])


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
