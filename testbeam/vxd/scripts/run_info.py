#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
This module contains several functions that help the setting up of the
analysis of the '16 DESY testbeam data, making use of the file
testbeam/vxd/data/elog_tbdesy_apr2016.csv (rev 28640).

More specifically, when providing a filename for a root file it gets for you:
- beam's energy
- is magnet ON or OFF ?
- telescope run number
- telescope data filename
- elog xml filename
- masking file with noisy strips for the run

This module also:
- prints info about quality of run


Giacomo Caria, g.caria@student.unimelb.edu.au
'''

################################################################################
# Imports
################################################################################

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

# #############  Helper tools to use this module's functions  #################


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


def find_runNo_line(csvFilename, runFilename):
    """Find the line of the csv file that contains all info about this run"""
    runNo = runNo_from_filename(runFilename)

    with open(csvFilename, 'rt') as q:

        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if str(runNo) in row[3]:
                return i


# #########################  Run info  #########################

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

                print('Run quality: PXD: ' + run_quality[0] + ', SVD: ' + run_quality[1] + ', Tel: ' + run_quality[2])


# #########################  Momentum info  #########################

def get_momentum(csvFilename, runFilename):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:
                energy = row[10]
                print('Beam energy is: ' + energy + ' GeV.')
                return energy


# #########################  Magnet info  #########################

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
                print('Magnet is On: ' + str(magnetBool))
                return magnetBool

# #########################  Elog xml file #########################


def get_elog_filename(csvFilename, runFilename, xmlPath):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:
                elogFilename = row[21]

    for root, dirs, files in os.walk(xmlPath):
        if elogFilename in files:
            print('xmlFile fount at: ' + xmlPath + elogFilename)
            return os.path.join(root, elogFilename)
        else:
            print("Couldn't find " + elogFilename + " in " + xmlPath)
            return


# #########################  Noisy strips file #########################

def get_mask_filename(runFilename, maskPath, det="SVD"):

    runNo = runNo_from_filename(runFilename)
    maskFilename = det + "_MaskFired_Run" + runNo + ".xml"

    for root, dirs, files in os.walk(maskPath):
        if maskFilename in files:
            print('maskFile fount at: ' + maskPath + maskFilename)
            return os.path.join(root, maskFilename)
        else:
            print("Couldn't find " + maskFilename + " in " + maskPath)
            return


# #########################  Telescope file #########################

def get_telRunNo(csvFilename, runFilename):

    line = find_runNo_line(csvFilename, runFilename)

    with open(csvFilename, 'rt') as q:
        reader = csv.reader(q, delimiter=',')

        for i, row in enumerate(reader):
            if i == line:

                telRunNo = row[5]
                if telRunNo.endswith('.0'):
                    telRunNo = telRunNo[:-2]
                print('telRunNo is: ' + telRunNo)
                return telRunNo


def get_tel_filename(csvFilename, runFilename, telPath):

    telRunNo = get_telRunNo(csvFilename, runFilename)

    # remove decimal part from telRunNo
    telFilename = 'run000' + telRunNo + '.raw'

    for root, dirs, files in os.walk(telPath):
        if telFilename in files:
            print('telFile fount at: ' + telPath + telFilename)
            return os.path.join(root, telFilename)
        else:
            print("Couldn't find " + telFilename + " in " + telPath)
            return
