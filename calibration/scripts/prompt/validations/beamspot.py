# -*- coding: utf-8 -*-

'''
Validation of the Beam Spot calibration
'''


import basf2
from prompt import ValidationSettings
import ROOT
from ROOT.Belle2 import ARICHCalibrationChecker
import sys
import subprocess
import math
import json

import matplotlib.pyplot as plt

import ROOT
import re
import os
from glob import glob
from math import sqrt
from math import frexp


#: Tells the automated system some details of this script
settings = ValidationSettings(name='Beam Spot values',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={})


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

    # import sys
    # sys.exit(0)
    arr = []

    fList = glob(path + '/*.root')
    for fName in fList:
        # print(fName)
        r = re.findall("[0-9]+", os.path.basename(fName))
        Id = int(r[0])
        # print(Id, runDict[Id])
        # continue
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

            arr.append((runDict[Id], tStart, tEnd, ip, ipe, sizeM))
            # print(ip, ipe, sizeM)

    arr = sorted(arr, key=lambda x: x[1])

    for e in arr:
        print(
            e[0][0],
            e[0][1],
            e[1],
            e[2],
            e[3][0],
            e[3][1],
            e[3][2],
            e[4][0],
            e[4][1],
            e[4][2],
            e[5][0],
            e[5][1],
            e[5][2],
            e[5][3],
            e[5][4],
            e[5][5])
    return arr


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    '''
    Run the validation.
    '''

    expert_config = json.loads(expert_config)

    # Run ROOT in batch mode.
    ROOT.gROOT.SetBatch(True)
    # And unset the stat box.
    ROOT.gStyle.SetOptStat(0)

    # Path to the database.txt file.
    outDir = f'{job_path}/BeamSpot/outputdb'
    database_file = outDir + '/database.txt'

    print(database_file)
    getBSvalues(outDir)

    file_name = 'plotFile.png'
    plt.plot([1, 2, 3], [3, 3, 4])
    plt.savefig(file_name)


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
