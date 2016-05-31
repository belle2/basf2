#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
This module contains several functions that help the setting up of the
analysis of the '16 DESY testbeam data, making use of the file
testbeam/vxd/data/elog_tbdesy_apr2016.csv (rev 28640).

More specifically, when providing a filename for a root file it gets:
- beam's energy
- is magnet ON or OFF ?
- telescope run number
- telescope data filename
- elog xml filename
- prints info about quality of run

Giacomo Caria, 31/05/16
'''

import os
import sys
import math
import numpy as np
import time
import csv
import re
from os import listdir
from os.path import isfile, join
import codecs


def get_file_header(csvFilename):

    with open(csvFilename, 'rt') as q:

        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == 0:
                columns_title = row
    print(columns_title)


def runNo_from_filename(s):

    first = '0'
    last = '.root'

    try:
        start = s.rindex(first) + len(first)
        end = s.rindex(last, start)
        return s[start:end]
    except ValueError:
        return ""


def get_elog_filename(csvFilename, runFilename, xmlpath):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:

                elogFilename = row[21]

    print('xml elog file fount at: ' + xmlpath + elogFilename)
    return xmlpath + elogFilename


def find_runNo_line(csvFilename, runFilename):

    runNo = runNo_from_filename(runFilename)

    with open(csvFilename, 'rt') as q:

        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if str(runNo) in row[3]:
                return i


def find_telFilename(telFilename, path):

    for root, dirs, files in os.walk(path):
        if telFilename in files:
            return os.path.join(root, telFilename)


def print_run_info(csvFilename, runFilename):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:

                if row[9] == 'OFF':
                    print('Beam was OFF for this run !! Try with different runNo.')
                    sys.exit()

                run_quality = []
                run_quality.append(row[16])
                run_quality.append(row[17])
                run_quality.append(row[18])

                if run_quality[0] == 'Bad':
                    print(' WARNING: Bad quality for PXD run')
                if run_quality[1] == 'Bad':
                    print(' WARNING: Bad quality for SVD run')
                if run_quality[2] == 'Bad':
                    print(' WARNING: Bad quality for Tel run')

                print('Run quality: PXD :' + run_quality[0] + ', SVD :' + run_quality[1] + ', Tel :' + run_quality[2])


def get_telescope_run(csvFilename, runFilename, path):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:

                telRunNo = row[5]
                # remove decimal part from telRunNo
                if telRunNo.endswith('.0'):
                    telRunNo = telRunNo[:-2]
                    telFileName = 'run000' + telRunNo + '.raw'
                print('tel runNo is: ' + telRunNo)
                print('telFile fount at: ' + find_telFilename(telFileName, path))
                return telFileName


def get_momentum(csvFilename, runFilename):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:
                energy = row[10]
                print('Beam energy is: ' + energy + ' GeV')
                return energy


def is_magnet_on(csvFilename, runFilename):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:

                magnetOn = row[11]

                if magnetOn == 'ON':
                    magnetBool = True
                elif magnetOn == 'OFF':
                    magnetBool = False
                print('magnet is On: ' + str(magnetBool))
                return magnetBool
